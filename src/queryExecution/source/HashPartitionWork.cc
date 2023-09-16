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

            // to load output page
            if (output == nullptr) {
                //use broadcastPageSize for broadcast join and hash partition join
                output = (char*)calloc(conf->getNetBroadcastPageSize(), 1);
                makeObjectAllocatorBlock(output, conf->getNetBroadcastPageSize(), true);
                myMaps = shuffler->createNewOutputContainer();
            }
            // get the vector corresponding to the i-th node
            // for each the map in the vector, we shuffle the map to the output page
            Record<Vector<Handle<Vector<Handle<Object>>>>>* record =
                (Record<Vector<Handle<Vector<Handle<Object>>>>>*)(page->getBytes());
            if (record != nullptr) {
                Handle<Vector<Handle<Vector<Handle<Object>>>>> objectsToShuffle =
                    record->getRootObject();
                Handle<Vector<Handle<Object>>>& objectToShuffle = (*objectsToShuffle)[id];
                Vector<Handle<Object>>& theOtherMaps = *objectToShuffle;
                for (size_t i = 0; i < theOtherMaps.size(); i++) {
                    if (theOtherMaps[i] != nullptr) {
		        Handle<JoinMap<Object>> theOtherMap = unsafeCast<JoinMap<Object>>(theOtherMaps[i]);
                        JoinMap<Object> mapToShuffle = *theOtherMap;
                        while (true) {
                            bool success = shuffler->writeOut(theOtherMaps[i], myMaps);
                            if (success == true) {

                                break;
                            } else {
                                getRecord(myMaps);
                                Record<Object>* myRecord = (Record<Object>*)output;
                                size_t numBytes = myRecord->numBytes();
                                char* sendBuffer = (char*)malloc(numBytes);
                                if (sendBuffer == nullptr) {
                                    std::cout << "Out of memory on heap" << std::endl;
                                    exit(-1);
                                }
                                memcpy(sendBuffer, output, numBytes);
                                
                                if (id != myNodeId) {
                                    makeObjectAllocatorBlock(128 * 1024, true);
                                    stage->sendData(communicator,
                                         sendBuffer,
                                         numBytes,
                                         jobStage->getSinkContext()->getDatabase(),
                                         jobStage->getSinkContext()->getSetName(),
                                         errMsg,
                                         numPages);
                                } else {
                                    makeObjectAllocatorBlock(128 * 1024, true);
                                    bool ret = proxy->pinBytes(jobStage->getSinkContext()->getDatabaseId(),
                                         jobStage->getSinkContext()->getTypeId(),
                                         jobStage->getSinkContext()->getSetId(),
                                         numBytes,
                                         sendBuffer,
                                         false);
                                    if (!ret) {
                                        std::cout << "Error: Failed to pin bytes" << std::endl;
                                    }
                                 }//if (id != myNodeId) {
                                 free(sendBuffer);
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
                        numMaps++;
                    } //if theOtherMaps[i] != nullpt<r
                }  // for (size_t i = 0; i < theOtherMaps.size(); i++) {

            }  // if (record != nullptr)
            // unpin the input page
            page->decRefCount();
            if (page->getRefCount() == 0) {
                page->freeContent();
            }
        }  // if(page != nullptr) {
    }      // while
    // send out the last page
    if (myMaps != nullptr) {
        getRecord(myMaps);
        Record<Object>* myRecord = (Record<Object>*)output;
        size_t numBytes = myRecord->numBytes();
        char* sendBuffer = (char*)malloc(numBytes);
        if (sendBuffer == nullptr) {
            std::cout << "Out of memory on heap" << std::endl;
            exit(-1);
        }
        memcpy(sendBuffer, output, numBytes);
        if (id != myNodeId) {
            //disable sending just for debugging, and we need to recover the below logic 
            makeObjectAllocatorBlock(128 * 1024, true);

            stage->sendData(communicator,
                sendBuffer,
                numBytes,
                jobStage->getSinkContext()->getDatabase(),
                jobStage->getSinkContext()->getSetName(),
                errMsg,
                numPages);
            
        } else {
            makeObjectAllocatorBlock(128 * 1024, true);
            bool ret = proxy->pinBytes(jobStage->getSinkContext()->getDatabaseId(),
                jobStage->getSinkContext()->getTypeId(),
                jobStage->getSinkContext()->getSetId(),
                numBytes,
                sendBuffer,
                false);
            if (!ret) {
                std::cout << "Error: Failed to pin bytes" << std::endl;
            }
        }
        free(sendBuffer);
        numPages++;
        myMaps = nullptr;
    }
    if (output != nullptr) {
        free(output);
        output = nullptr;
    }
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
}

}

#endif
