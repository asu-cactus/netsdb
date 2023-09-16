
#ifndef PIPELINE_H
#define PIPELINE_H

#include "ComputeSource.h"
#include "ComputeSink.h"
#include "UseTemporaryAllocationBlock.h"
#include "Handle.h"
#include <queue>

#ifndef MIN_BATCH_SIZE
#define MIN_BATCH_SIZE 1
#endif


#ifndef DELAY_VALUE
#define DELAY_VALUE 4
#endif

namespace pdb {

// this is used to buffer unwritten pages
struct MemoryHolder {

    // the output vector that this guy stores
    Handle<Object> outputSink = nullptr;

    // his memory
    void* location;
    size_t size;



    // the iteration where he was last written...
    // we use this beause we canot delete
    int iteration;

    size_t getSize() { return size; }

    void setIteration(int iterationIn) {
        if (outputSink != nullptr)
            getRecord(outputSink);
        iteration = iterationIn;
    }

    MemoryHolder(std::pair<void*, size_t> buildMe) {
        location = buildMe.first;
        size = buildMe.second;
        makeObjectAllocatorBlock(location, buildMe.second, true);
        outputSink = nullptr;
    }
};

typedef std::shared_ptr<MemoryHolder> MemoryHolderPtr;

// this is a prototype for the pipeline
class Pipeline {

private:
    // this is a function that the pipeline calls to obtain a new page to
    // write output to.  The function returns a pair.  The first item in
    // the pair is the page, the second is the number of bytes in the page
    std::function<std::pair<void*, size_t>()> getNewPage;

    // this is a function that the pipeline calls to write back a page.
    // The first arg is the page to write back (and free), and the second
    // is the size of the page
    std::function<void(void*)> writeBackPage;

    // this is a function that the pipieline calls to free a page, without
    // writing it back (because it has no useful data)
    std::function<void(void*)> discardPage;

    // this is the source of data in the pipeline
    ComputeSourcePtr dataSource;

    // this is where the pipeline goes to write the data
    ComputeSinkPtr dataSink;

    // here is our pipeline
    std::vector<ComputeExecutorPtr> pipeline;

    // and here is all of the pages we've not yet written back
    std::queue<MemoryHolderPtr> unwrittenPages;

    int delay = DELAY_VALUE;

public:

    int id;

    // the first argument is a function to call that gets a new output page...
    // the second arguement is a function to call that deals with a full output page
    // the third argument is the iterator that will create TupleSets to process
    Pipeline(std::function<std::pair<void*, size_t>()> getNewPage,
             std::function<void(void*)> discardPage,
             std::function<void(void*)> writeBackPage,
             ComputeSourcePtr dataSource,
             ComputeSinkPtr tupleSink)
        : getNewPage(getNewPage),
          writeBackPage(writeBackPage),
          discardPage(discardPage),
          dataSource(dataSource),
          dataSink(tupleSink) {}

    // adds a stage to the pipeline
    void addStage(ComputeExecutorPtr addMe) {
        pipeline.push_back(addMe);
    }

    ~Pipeline() {

        // kill all of the pipeline stages
        while (pipeline.size())
            pipeline.pop_back();
        
        // first, reverse the queue so we go oldest to newest
        // this ensures that everything is deleted in the reverse order that it was created
        std::vector<MemoryHolderPtr> reverser;
        while (unwrittenPages.size() > 0) {
            reverser.push_back(unwrittenPages.front());
            unwrittenPages.pop();
        }

        while (reverser.size() > 0) {
            unwrittenPages.push(reverser.back());
            reverser.pop_back();
        }
        

        // write back all of the pages
        cleanPages(999999999);

        if (unwrittenPages.size() != 0)
            std::cout
                << "This is bad: in destructor for pipeline, still some pages with objects!!\n";
    }

    // writes back any unwritten pages
    void cleanPages(int iteration) {

        // take care of getting rid of any pages... but only get rid of those from two iterations
        // ago...
        // pages from the last iteration may still have pointers into them
        while (unwrittenPages.size() > 0 && iteration > unwrittenPages.front()->iteration + delay) {
            // in this case, the page did not have any output data written to it... it only had
            // intermediate results, and so we will just discard it
            if (unwrittenPages.front()->outputSink == nullptr) {
                std::cout << id << ": WARNING: we find an output page that has no container in it, which is bad, so we create one" << std::endl;
                unwrittenPages.front()->outputSink = dataSink->createNewOutputContainer();
                getRecord(unwrittenPages.front()->outputSink); 
                if (getNumObjectsInAllocatorBlock(unwrittenPages.front()->location) != 0) {

                    // this is bad... there should not be any objects here because this memory
                    // chunk does not store an output vector
                    emptyOutContainingBlock(unwrittenPages.front()->location);

                }
                discardPage(unwrittenPages.front()->location);
                unwrittenPages.pop();

                // in this case, the page DID have some data written to it
            } else {
                // and force the reference count for this guy to go to zero
                unwrittenPages.front()->outputSink.emptyOutContainingBlock();

                // OK, because we will have invalidated the current object allocator block, we need
                // to
                // create a new one, or this could cause a lot of problems!!
                if (iteration == 999999999)
                    makeObjectAllocatorBlock(1024, true);
                // make sure he is written
                writeBackPage(unwrittenPages.front()->location);
                
                // and get ridda him
                unwrittenPages.pop();
            }
        }
    }

    // runs the pipeline
    void run() {

        // this is where we are outputting all of our results to
        MemoryHolderPtr myRAM = std::make_shared<MemoryHolder>(getNewPage());

        if (myRAM->location == nullptr) {
            std::cout << "ERROR: insufficient memory in heap" << std::endl;
            exit(1);
        }
        myRAM->outputSink = dataSink->createNewOutputContainer();

        // and here is the chunk
        TupleSetPtr curChunk = nullptr;

        // the iteration counter
        int iteration = 0;

        // while there is still data
        // Jia Note: dataSource->getNextTupleSet() can throw exception for certain data sources like
        // MapTupleSetIterator
        while (true) {

           try {
               curChunk = dataSource->getNextTupleSet();
           } catch (NotEnoughSpace& n) {
               curChunk = nullptr;
               myRAM->setIteration(iteration);
               unwrittenPages.push(myRAM);
               iteration++;
               cleanPages(iteration);
               myRAM = std::make_shared<MemoryHolder>(getNewPage());
               if (myRAM->location == nullptr) {
                   std::cout << id << ": ERROR: insufficient memory in heap" << std::endl;
                   return;
               }
               myRAM->outputSink = dataSink->createNewOutputContainer();
               curChunk = dataSource->getNextTupleSet();
           }

           if (curChunk == nullptr) {
               std::cout << id << ": WARNING: get an empty chunk in pipeline" << std::endl;
               break;
           }
                      
            // go through all of the pipeline stages
           for (ComputeExecutorPtr& q : pipeline) {

                try {
                    curChunk = q->process(curChunk);

                } catch (NotEnoughSpace& n) {
                    // and get a new page
                    myRAM->setIteration(iteration);
                    unwrittenPages.push(myRAM);
                    iteration++;
                    cleanPages(iteration);
                    myRAM = std::make_shared<MemoryHolder>(getNewPage());
                    if (myRAM->location == nullptr) {
                        std::cout << "ERROR: insufficient memory in heap or the corresponding partition sink is used up" << std::endl;
                        return;
                    }
                    myRAM->outputSink = dataSink->createNewOutputContainer();

                    // then try again
                    try {
                        curChunk = q->process(curChunk);
                    } catch (NotEnoughSpace& n) {
                        std::cout << id << ": curChunk contains " << curChunk->getNumRows(0) << " rows" 
                                  << std::endl;
                        std::cout << id << ": Pipeline Error: Batch processing memory exceeds page size "
                                     "for executor type: "
                                  << q->getType() << ", consider to reduce batch size or increase page size. Current batch size is "<<dataSource->getChunkSize() << ", current page size is "<< myRAM->getSize() << std::endl; 
                        exit(1);
                    }
                }
           }

           bool end = false;
           while (!end) {
                try {

                   if (myRAM->outputSink == nullptr) {
                       myRAM->outputSink = dataSink->createNewOutputContainer();
                   }
                   dataSink->writeOut(curChunk, myRAM->outputSink);
                   end = true;                    

                } catch (NotEnoughSpace& n) {

                    // again, we ran out of RAM here, so write back the page and then create a new
                    // output page
                    myRAM->setIteration(iteration);
                    unwrittenPages.push(myRAM);
                    iteration++;
                    cleanPages(iteration);
                    myRAM = std::make_shared<MemoryHolder>(getNewPage());
                    if (myRAM->location == nullptr) {
                        std::cout << "ERROR: insufficient memory in heap or the corresponding partition sink is used up" << std::endl;
                        return;
                    }
                 }
           }
            // lastly, write back all of the output pages
        }

        // set the iteration
        myRAM->setIteration(iteration);
        // and remember the page
        unwrittenPages.push(myRAM);
        iteration++;

    }
};

typedef std::shared_ptr<Pipeline> PipelinePtr;
}

#endif
