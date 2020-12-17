#ifndef HASH_PARTITION_WORK_CC
#define HASH_PARTITION_WORK_CC

#include "PDBDebug.h"
#include "HashPartitionWork.h"
#include "PDBPage.h"
#include "PDBCommunicator.h"
#include "SimpleRequestResult.h"
#include "UseTemporaryAllocationBlock.h"
#include "DataProxy.h"
#include "string.h"
#include <thread>
#include <sstream>


namespace pdb {

HashPartitionWork :: HashPartitionWork(int id, PageCircularBufferIteratorPtr iter,  PipelineStage * stage, int& counter) 
    : counter(counter) {
    this->id = id;
    this->iter = iter;
    this->stage = stage;
    this->jobStage = stage->getJobStage();
    this->conf = stage->getConf();
    pthread_mutex_init(&connection_mutex, nullptr);
}

HashPartitionWork :: ~HashPartitionWork() {
    pthread_mutex_destroy(&connection_mutex);
}

void HashPartitionWork :: execute(PDBBuzzerPtr callerBuzzer) {

    UseTemporaryAllocationBlock tempBlock{32 * 1024 * 1024};

    PDBLoggerPtr logger = make_shared<PDBLogger>("hashPartitionWorks.log");

    // to combine data for node-i
    std::string errMsg;
    
    // create the data proxy
    DataProxyPtr proxy = nullptr;
    NodeID myNodeId = jobStage->getNodeId();
    if (id == myNodeId) {
        proxy = stage->createProxy(id, connection_mutex, errMsg);
    }

    // get the i-th address
    std::string address = this->jobStage->getIPAddress(id);
    PDB_COUT << "address = " << address << std::endl;

    // get the i-th port
    int port = this->jobStage->getPort(id);
    PDB_COUT << "port = " << port << std::endl;

    PDBCommunicatorPtr communicator = std::make_shared<PDBCommunicator>();
    communicator->connectToInternetServer(logger, port, address, errMsg);

    // get join computation
    PDB_COUT << id << ": to get compute plan" << std::endl;
    Handle<ComputePlan> plan = this->jobStage->getComputePlan();
    plan->nullifyPlanPointer();
    PDB_COUT << id << ": to deep copy ComputePlan object" << std::endl;
    Handle<ComputePlan> newPlan = deepCopyToCurrentAllocationBlock<ComputePlan>(plan);
    std::string sourceTupleSetSpecifier = jobStage->getSourceTupleSetSpecifier();
    std::string targetTupleSetSpecifier = jobStage->getTargetTupleSetSpecifier();
    std::string targetSpecifier = jobStage->getTargetComputationSpecifier();
     
    // get shuffler
    SinkShufflerPtr shuffler = newPlan->getShuffler(
         sourceTupleSetSpecifier, targetTupleSetSpecifier, targetSpecifier);
    shuffler->setNodeId(id);
    PageCircularBufferIteratorPtr myIter = iter;
    int numPages = 0;
    int numMaps = 0;

    // set non-reuse policy
    char* output = nullptr;
    char* buffer = nullptr;
    Handle<Object> myMaps = nullptr;
    while (myIter->hasNext()) {
        PDBPagePtr page = myIter->next();
        if (page != nullptr) {
            std::cout << "worker-" << id << ": received a non-empty page" << std::endl;
            // to load output page
            if (output == nullptr) {
                //use broadcastPageSize for broadcast join and hash partition join
                output = (char*)calloc(conf->getNetBroadcastPageSize()-sizeof(size_t), 1);
                std::cout << "Alloc output buffer size for repartition data: " <<
                    conf->getNetBroadcastPageSize()-sizeof(size_t) << std::endl;
                makeObjectAllocatorBlock(output, conf->getNetBroadcastPageSize()-sizeof(size_t), true);
                std::cout << getAllocator().printCurrentBlock() << std::endl;
                myMaps = shuffler->createNewOutputContainer();
            }
            // get the vector corresponding to the i-th node
            // for each the map in the vector, we shuffle the map to the output page
            Record<Vector<Handle<Vector<Handle<Object>>>>>* record =
                (Record<Vector<Handle<Vector<Handle<Object>>>>>*)(page->getBytes());
            if (record != nullptr) {
                std::cout << "worker-" << id << ": received a non-empty record" << std::endl;
                Handle<Vector<Handle<Vector<Handle<Object>>>>> objectsToShuffle =
                    record->getRootObject();
                if (objectsToShuffle != nullptr) {
                     std::cout << id << ": number of vectors in the record: " << objectsToShuffle->size() << std::endl;
                }
                Handle<Vector<Handle<Object>>>& objectToShuffle = (*objectsToShuffle)[id];
                Vector<Handle<Object>>& theOtherMaps = *objectToShuffle;
                std::cout << "number of sub vectors for id="<< id << ": " << theOtherMaps.size() << std::endl; 
                for (unsigned int i = 0; i < theOtherMaps.size(); i++) {
                    if (theOtherMaps[i] != nullptr) {
                        bool success = shuffler->writeOut(theOtherMaps[i], myMaps);
                        if (success == false) {
                            std::cout << id << ": output page is full, send it out" << std::endl;
                            getRecord(myMaps);
                            Record<Object>* myRecord = (Record<Object>*)output;
                            size_t numBytes = myRecord->numBytes();
                            std::cout << id << ": Alloc output buffer is full with size: " << numBytes << std::endl;
                            char* sendBuffer = (char*)malloc(numBytes);
                            if (sendBuffer == nullptr) {
                                std::cout << "Out of memory on heap" << std::endl;
                                exit(-1);
                            }
                            memcpy(sendBuffer, output, numBytes);
                            if (id != myNodeId) {
                                std::cout << getAllocator().printCurrentBlock()
                                                  << std::endl;
                                makeObjectAllocatorBlock(128 * 1024, true);
                                std::cout << id << ": sendData to "<< jobStage->getSinkContext()->getDatabase()
                                          << ":" << jobStage->getSinkContext()->getSetName() << std::endl;
                                stage->sendData(communicator,
                                         sendBuffer,
                                         numBytes,
                                         jobStage->getSinkContext()->getDatabase(),
                                         jobStage->getSinkContext()->getSetName(),
                                         errMsg);
                                std::cout << id << ": sent data to "<< jobStage->getSinkContext()->getDatabase()
                                          << ":" << jobStage->getSinkContext()->getSetName() << std::endl;
                             } else {
                                makeObjectAllocatorBlock(128 * 1024, true);
                                std::cout << id << ": pinBytes to " << jobStage->getSinkContext()->getDatabaseId() 
                                          << ":" << jobStage->getSinkContext()->getTypeId()
                                          << ":" << jobStage->getSinkContext()->getSetId();
                                bool ret = proxy->pinBytes(jobStage->getSinkContext()->getDatabaseId(),
                                         jobStage->getSinkContext()->getTypeId(),
                                         jobStage->getSinkContext()->getSetId(),
                                         numBytes,
                                         sendBuffer,
                                         false);
                                std::cout << id << ": pined bytes to " << jobStage->getSinkContext()->getDatabaseId()
                                          << ":" << jobStage->getSinkContext()->getTypeId()
                                          << ":" << jobStage->getSinkContext()->getSetId();
                                if (!ret) {
                                    std::cout << "Error: Failed to pin bytes" << std::endl;
                                }
                             }
                             std::cout << id << ": to free the " << numPages << "-th sendBuffer" << std::endl;
                             free(sendBuffer);
                             std::cout << id << ": freed the " << numPages << "-th sendBuffer" << std::endl;
                             numPages++;
                             // free the output page and reload a new output page
                             myMaps = nullptr;
                             buffer = (char*)calloc(conf->getNetBroadcastPageSize()-sizeof(size_t), 1);
                             makeObjectAllocatorBlock(buffer, conf->getNetBroadcastPageSize()-sizeof(size_t), true);
                             // redo for current map;
                             myMaps = shuffler->createNewOutputContainer();
                             bool res = shuffler->writeOut(theOtherMaps[i], myMaps);
                             if (res == false) {
                                 std::cout << id << ":Error: we do not have room to write all resting data out" << std::endl;
                             } else {
                                 std::cout << id << ": we successfully write the resting of the map out" << std::endl;
                             }
                        }//if (success == false)
                    } //if theOtherMaps[i] != nullpt<r
                    std::cout << id << ": HashPartitioned the "<< i << "-th map" << std::endl;
                    numMaps++;
                    if ((output != nullptr) && (buffer != nullptr) && (output != buffer)) {
                         std::cout << id <<": to free the output buffer" << std::endl;
                         free(output);
                         std::cout << id <<": freed the output buffer" << std::endl;
                         output = buffer;
                    }
                }  // for

            }  // if (record != nullptr)
            // unpin the input page
            page->decRefCount();
            if (page->getRefCount() == 0) {
                std::cout << id << ": to free shuffle input page with refCount=" << page->getRefCount() << std::endl;
                page->freeContent();
                std::cout << id << ": freed shuffle input page with refCount=" << page->getRefCount() << std::endl;
            }
        }  // if
    }      // while
    // send out the last page
    numPages++;
    if (myMaps != nullptr) {
        getRecord(myMaps);
        Record<Object>* myRecord = (Record<Object>*)output;
        size_t numBytes = myRecord->numBytes();
        std::cout << id << ": Alloc output buffer not full with size: " << numBytes << std::endl;
        char* sendBuffer = (char*)malloc(numBytes);
        if (sendBuffer == nullptr) {
            std::cout << "Out of memory on heap" << std::endl;
            exit(-1);
        }
        memcpy(sendBuffer, output, numBytes);
        if (id != myNodeId) {
            makeObjectAllocatorBlock(128 * 1024, true);
            std::cout << id <<": sendData" << std::endl;
            stage->sendData(communicator,
                sendBuffer,
                numBytes,
                jobStage->getSinkContext()->getDatabase(),
                jobStage->getSinkContext()->getSetName(),
                errMsg);
            std::cout << id <<": sent data" << std::endl;
        } else {
            makeObjectAllocatorBlock(128 * 1024, true);
            std::cout << id <<": pinBytes" << std::endl;
            bool ret = proxy->pinBytes(jobStage->getSinkContext()->getDatabaseId(),
                jobStage->getSinkContext()->getTypeId(),
                jobStage->getSinkContext()->getSetId(),
                numBytes,
                sendBuffer,
                false);
            std::cout << id <<": pinned bytes" << std::endl;
            if (!ret) {
                std::cout << "Error: Failed to pin bytes" << std::endl;
            }
        }
        std::cout << id << ": to free the "<< numPages<<"-th send buffer" << std::endl;
        free(sendBuffer);
        std::cout << id << ": freed the "<< numPages<<"-th send buffer" << std::endl;
        numPages++;
        myMaps = nullptr;
    }
    if (output != nullptr) {
        std::cout << id << ": to free the final output" << std::endl;
        free(output);
        output = nullptr;
    }
    std::cout << id << ": HashPartitioned " << numPages << " pages to address: " << address
                      << std::endl;
    std::cout << id << ":" << numMaps << " maps are written in total for partition-" << id << std::endl;
    if (id != myNodeId) {
        makeObjectAllocatorBlock(128 * 1024, true);
        stage->sendData(communicator,
                 nullptr,
                 conf->getNetBroadcastPageSize(),
                 jobStage->getSinkContext()->getDatabase(),
                 jobStage->getSinkContext()->getSetName(),
                 errMsg);
    }
    callerBuzzer->buzz(PDBAlarm::WorkAllDone, counter);
    std::cout << "finished " << id << "-th HashPartition work on " << myNodeId << std::endl;
}

}

#endif
