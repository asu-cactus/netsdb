#ifndef HASH_PARTITION_WORK_CC
#define HASH_PARTITION_WORK_CC

#include "PDBDebug.h"
#include "HashPartitionWork.h"
#include "PDBPage.h"
#include "PDBCommunicator.h"
#include "SimpleRequestResult.h"
#include "UseTemporaryAllocationBlock.h"
#include "DataProxy.h"
#include "JoinComp.h"
#include "string.h"
#include <openssl/md5.h>
#include <thread>
#include <sstream>


namespace pdb {

HashPartitionWork :: HashPartitionWork(int id, PageCircularBufferIteratorPtr iter,  PipelineStage * stage, atomic_int& counter) 
    : counter(counter) {
    this->id = id;
    this->iter = iter;
    this->stage = stage;
    this->jobStage = stage->getJobStage();
    this->conf = stage->getConf();
    this->logger = std::make_shared<PDBLogger>("repartition"+std::to_string(id)+".log");
    pthread_mutex_init(&connection_mutex, nullptr);
}

HashPartitionWork :: ~HashPartitionWork() {
    pthread_mutex_destroy(&connection_mutex);
}

void HashPartitionWork :: execute(PDBBuzzerPtr callerBuzzer) {

    UseTemporaryAllocationBlock tempBlock{32 * 1024 * 1024};


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
            logger->writeInt(id);
            logger->writeLn(": received a non-empty page");            

            // to load output page
            if (output == nullptr) {
                //use broadcastPageSize for broadcast join and hash partition join
                output = (char*)calloc(conf->getNetBroadcastPageSize(), 1);
                std::cout << id <<": Alloc output buffer size for repartition data: " <<
                    conf->getNetBroadcastPageSize() << std::endl;
                logger->writeLn(": Alloc output buffer size for repartition data: ");
                logger->writeInt(conf->getNetBroadcastPageSize());
                makeObjectAllocatorBlock(output, conf->getNetBroadcastPageSize(), true);
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
                     logger->writeLn(": number of vectors in the record: ");
                     logger->writeInt(objectsToShuffle->size());
                }
                Handle<Vector<Handle<Object>>>& objectToShuffle = (*objectsToShuffle)[id];
                Vector<Handle<Object>>& theOtherMaps = *objectToShuffle;
                std::cout << "number of sub vectors for id="<< id << ": " << theOtherMaps.size() << std::endl;
                logger->writeLn("number of sub vectors for this node=");
                logger->writeInt(theOtherMaps.size()); 
                for (size_t i = 0; i < theOtherMaps.size(); i++) {
                    if (theOtherMaps[i] != nullptr) {
                        while (true) {
                            bool success = shuffler->writeOut(theOtherMaps[i], myMaps);
                            if (success == true) {
                                break;
                            } else {
                                std::cout << id << ": output page is full, send it out" << std::endl;
                                logger->writeLn(": output page is full, send it out");
                                Handle<Vector<Handle<JoinMap<JoinTuple<Object, char[0]>>>>> mapsToPrint 
                                    = unsafeCast<Vector<Handle<JoinMap<JoinTuple<Object, char[0]>>>>>(myMaps);
                                for (int j = 0; j < mapsToPrint->size(); j++) {
                                    ((*mapsToPrint)[j])->print();
                                }
                                getRecord(myMaps);
                                Record<Object>* myRecord = (Record<Object>*)output;
                                size_t numBytes = myRecord->numBytes();
                                std::cout << id << ": Alloc output buffer is full with size: " << numBytes << std::endl;
                                logger->writeLn(": Alloc output buffer is full with size: ");
                                logger->writeInt(numBytes);
                                char* sendBuffer = (char*)malloc(numBytes);
                                if (sendBuffer == nullptr) {
                                    std::cout << "Out of memory on heap" << std::endl;
                                    exit(-1);
                                }
                                memcpy(sendBuffer, output, numBytes);
                                
                                if (id != myNodeId) {
                                    //commented for debugging, needs to recover)
                                    std::cout << getAllocator().printCurrentBlock()
                                                  << std::endl;
                                    makeObjectAllocatorBlock(128 * 1024, true);
                                    std::cout << id << ": sendData to "<< jobStage->getSinkContext()->getDatabase()
                                          << ":" << jobStage->getSinkContext()->getSetName() << std::endl;
                                    /*MD5_CTX md5;
                                    MD5_Init(&md5);
                                    MD5_Update(&md5, sendBuffer, numBytes);
                                    unsigned char* output = nullptr;
                                    MD5_Final(output, &md5);
                                    std::cout << "Hash sent: " << output << std::endl;
                                    logger->writeLn("Hash sent: "+string((char *)output));*/
                                    stage->sendData(communicator,
                                         sendBuffer,
                                         numBytes,
                                         jobStage->getSinkContext()->getDatabase(),
                                         jobStage->getSinkContext()->getSetName(),
                                         errMsg,
                                         numPages);
                                    std::cout << id << ": sent data to "<< jobStage->getSinkContext()->getDatabase()
                                          << ":" << jobStage->getSinkContext()->getSetName() << std::endl;
                                    logger->writeLn("sent data");    
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
                                    logger->writeLn("pined bytes");
                                    if (!ret) {
                                        std::cout << "Error: Failed to pin bytes" << std::endl;
                                    }
                                 }//if (id != myNodeId) {
                                 std::cout << id << ": to free the " << numPages << "-th sendBuffer" << std::endl;
                                 free(sendBuffer);
                                 std::cout << id << ": freed the " << numPages << "-th sendBuffer" << std::endl;
                                 logger->writeLn(": freed the ");
                                 logger->writeInt(numPages);
                                 logger->writeLn( "-th sendBuffer");
                                 numPages++;
                                 // free the output page and reload a new output page
                                 char * buffer = (char*)calloc(conf->getNetBroadcastPageSize(), 1);
                                 makeObjectAllocatorBlock(buffer, conf->getNetBroadcastPageSize(), true);
                                 // redo for current map;
                                 myMaps = shuffler->createNewOutputContainer();
                                 free(output);
                                 output = buffer; 
                             }//if (success == true) { 
                        }//while (true)
                        std::cout << id << ": HashPartitioned the "<< i << "-th map" << std::endl;
                        logger->writeLn(": HashPartitioned the ");
                        logger->writeInt(i);
                        logger->writeLn("-th map");
                        numMaps++;
                    } //if theOtherMaps[i] != nullpt<r
                }  // for (size_t i = 0; i < theOtherMaps.size(); i++) {

            }  // if (record != nullptr)
            // unpin the input page
            page->decRefCount();
            if (page->getRefCount() == 0) {
                std::cout << id << ": to free shuffle input page with refCount=" << page->getRefCount() << std::endl;
                page->freeContent();
                std::cout << id << ": freed shuffle input page with refCount=" << page->getRefCount() << std::endl;
                logger->writeLn(": freed shuffle input page with refCount=");
                logger->writeInt(page->getRefCount());
            }
        }  // if(page != nullptr) {
    }      // while
    // send out the last page
    if (myMaps != nullptr) {
        Handle<Vector<Handle<JoinMap<JoinTuple<Object, char[0]>>>>> mapsToPrint
           = unsafeCast<Vector<Handle<JoinMap<JoinTuple<Object, char[0]>>>>>(myMaps);
        /*for (int j = 0; j < mapsToPrint->size(); j++) {
           ((*mapsToPrint)[j])->print();
        }*/
        getRecord(myMaps);
        Record<Object>* myRecord = (Record<Object>*)output;
        size_t numBytes = myRecord->numBytes();
        std::cout << id << ": Alloc output buffer not full with size: " << numBytes << std::endl;
        logger->writeLn(": Alloc output buffer not full with size: ");
        logger->writeInt(numBytes);
        char* sendBuffer = (char*)malloc(numBytes);
        if (sendBuffer == nullptr) {
            std::cout << "Out of memory on heap" << std::endl;
            exit(-1);
        }
        memcpy(sendBuffer, output, numBytes);
        if (id != myNodeId) {
            //disable sending just for debugging, and we need to recover the below logic 
            makeObjectAllocatorBlock(128 * 1024, true);
            std::cout << id <<": sendData" << std::endl;

           /* MD5_CTX md5;
            MD5_Init(&md5);
            MD5_Update(&md5, sendBuffer, numBytes);
            unsigned char output[MD5_DIGEST_LENGTH];
            MD5_Final(output, &md5);
            std::cout << "Hash sent: " << output << std::endl;
            logger->writeLn("Hash sent: "+string((char *)output));
            */
            stage->sendData(communicator,
                sendBuffer,
                numBytes,
                jobStage->getSinkContext()->getDatabase(),
                jobStage->getSinkContext()->getSetName(),
                errMsg,
                numPages);
            
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
            logger->writeLn("pinned final bytes");
            if (!ret) {
                std::cout << "Error: Failed to pin bytes" << std::endl;
            }
        }
        std::cout << id << ": to free the "<< numPages<<"-th send buffer" << std::endl;
        free(sendBuffer);
        std::cout << id << ": freed the "<< numPages<<"-th send buffer" << std::endl;
        logger->writeLn(": freed the ");
        logger->writeInt(numPages);
        logger->writeLn("-th send buffer");
        numPages++;
        myMaps = nullptr;
    }
    if (output != nullptr) {
        std::cout << id << ": to free the final output" << std::endl;
        free(output);
        logger->writeLn(": freed final output");
        output = nullptr;
    }
    std::cout << id << ": HashPartitioned " << numPages << " pages to address: " << address
                      << std::endl;
    logger->writeLn(": HashPartitioned ");
    logger->writeInt(numPages);
    logger->writeLn(" pages to address: ");
    logger->writeLn(string(address));
    std::cout << id << ":" << numMaps << " maps are written in total for partition-" << id << std::endl;
    logger->writeInt(numMaps);
    logger->writeLn(" maps are written in total for partition-");
    logger->writeInt(id);
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
