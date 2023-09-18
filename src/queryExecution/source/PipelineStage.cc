#ifndef PIPELINE_STAGE_CC
#define PIPELINE_STAGE_CC

#include <openssl/md5.h>
#include "AbstractAggregateComp.h"
#include "StorageAddData.h"
#include "StorageAddObjectInLoop.h"
#include "ComputePlan.h"
#include "ScanUserSet.h"
#include "SelectionComp.h"
#include "MultiSelectionComp.h"
#include "PDBDebug.h"
#include "PipelineStage.h"
#include "PageCircularBufferIterator.h"
#include "DataProxy.h"
#include "PageScanner.h"
#include "PageCircularBufferIterator.h"
#include "InterfaceFunctions.h"
#include "HermesExecutionServer.h"
#include "GenericWork.h"
#include "SetSpecifier.h"
#include "UseTemporaryAllocationBlock.h"
#include "Configuration.h"
#include "ClusterAggregateComp.h"
#include "SharedHashSet.h"
#include "JoinComp.h"
#include "SimpleSendObjectRequest.h"
#include "SimpleSendBytesRequest.h"
#include "ShuffleSink.h"
#include "HashPartitionWork.h"
#include "PartitionComp.h"
#ifdef ENABLE_COMPRESSION
#include <snappy.h>
#endif

#include <fstream>


#ifndef DEFAULT_HASH_SET_SIZE
  #define DEFAULT_HASH_SET_SIZE ((size_t)(2)*(size_t)(1024)*(size_t)(1024)*(size_t)(1024))
#endif


namespace pdb {

static size_t tunedHashPageSize;

PipelineStage::~PipelineStage() {
    this->jobStage = nullptr;
    this->nodeIds.clear();
}

PipelineStage::PipelineStage(Handle<TupleSetJobStage> stage,
                             SharedMemPtr shm,
                             PDBLoggerPtr logger,
                             ConfigurationPtr conf,
                             NodeID nodeId,
                             size_t batchSize,
                             int numThreads) {
    this->jobStage = stage;
    this->batchSize = batchSize;
    this->numThreads = numThreads;
    this->nodeId = nodeId;
    this->logger = logger;
    this->conf = conf;
    this->shm = shm;
    this->id = 0;
    int numNodes = this->jobStage->getNumNodes();
    for (int i = 0; i < numNodes; i++) {
        nodeIds.push_back(i);
    }
}


Handle<TupleSetJobStage>& PipelineStage::getJobStage() {
    return jobStage;
}

ConfigurationPtr PipelineStage::getConf() {
    return conf;
}

int PipelineStage::getNumThreads() {
    return this->numThreads;
}

// send repartitioned data to a remote node
bool PipelineStage::storeShuffleData(Handle<Vector<Handle<Object>>> data,
                                     std::string databaseName,
                                     std::string setName,
                                     std::string address,
                                     int port,
                                     std::string& errMsg) {
    if (port <= 0) {
        port = conf->getPort();
    }
    std::cout << "store shuffle data to address=" << address << " and port=" << port
              << ", with size = " << data->size() << " to database=" << databaseName
              << " and set=" << setName << " and type = IntermediateData" << std::endl;
    return simpleSendDataRequest<StorageAddData, Handle<Object>, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        [&](Handle<SimpleRequestResult> result) {
            if (result != nullptr)
                if (!result->getRes().first) {
                    logger->error("Error sending data: " + result->getRes().second);
                    errMsg = "Error sending data: " + result->getRes().second;
                }
            return true;
        },
        data,
        databaseName,
        setName,
        "IntermediateData",
        false,
        false);
}

bool PipelineStage::storeCompressedShuffleData(char* bytes,
                                               size_t numBytes,
                                               std::string databaseName,
                                               std::string setName,
                                               std::string address,
                                               int port,
                                               std::string& errMsg) {
    if (port <= 0) {
        port = conf->getPort();
    }
    std::cout << "store shuffle data to address=" << address << " and port=" << port
              << ", with compressed byte size = " << numBytes << " to database=" << databaseName
              << " and set=" << setName << " and type = IntermediateData" << std::endl;
    return simpleSendBytesRequest<StorageAddData, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        [&](Handle<SimpleRequestResult> result) {
            if (result != nullptr)
                if (!result->getRes().first) {
                    logger->error("Error sending data: " + result->getRes().second);
                    errMsg = "Error sending data: " + result->getRes().second;
                }
            return true;
        },
        bytes,
        numBytes,
        databaseName,
        setName,
        "IntermediateData",
        false,
        false,
        true);
}

// broadcast data
bool PipelineStage::sendData(PDBCommunicatorPtr conn,
                             char* data,
                             size_t size,
                             std::string databaseName,
                             std::string setName,
                             std::string& errMsg,
                             int counter) {
    bool success;
    if (data != nullptr) {
#ifdef DEBUG_SHUFFLING
        // write the data to a test file
        std::string fileName =
            jobStage->getJobId() + "_" + std::to_string(jobStage->getStageId()) + "_shuffle-sent"+std::to_string(counter);
        FILE* myFile = fopen(fileName.c_str(), "w");
        fwrite(data, 1, size, myFile);
        fclose(myFile);
#endif

        Handle<StorageAddObjectInLoop> request = makeObject<StorageAddObjectInLoop>(
            databaseName, setName, "IntermediateData", false, false);
        conn->sendObject(request, errMsg);
#ifdef ENABLE_COMPRESSION
        size_t maxSize = snappy::MaxCompressedLength(size);
        char* compressedBytes = new char[maxSize];
        if (compressedBytes == nullptr) {
           std::cout << "PipelineStage.cc: failed to allocate memory with size=" << size << std::endl;
           exit(1);
        }
        memset(compressedBytes, 0, maxSize);
        size_t compressedSize = 0;
        snappy::RawCompress((char*)data, size, compressedBytes, &compressedSize);
        errMsg = "";
        conn->sendBytes(compressedBytes, compressedSize, errMsg);
        delete[] compressedBytes;
#else
        conn->sendBytes(data, size, errMsg);
#endif
    } else {
        Handle<StorageAddObjectInLoop> request = makeObject<StorageAddObjectInLoop>();
        request->setLoopEnded();
        conn->sendObject(request, errMsg);
    }
    Handle<SimpleRequestResult> result = conn->getNextObject<SimpleRequestResult>(success, errMsg);
    return true;
}

// tuning the backend circular buffer size
size_t PipelineStage::getBackendCircularBufferSize(bool& success, std::string& errMsg) {

    int backendCircularBufferSize = 1;
    if (conf->getShmSize() / conf->getPageSize() - 2 <
        2 + 2 * numThreads + backendCircularBufferSize) {
        success = false;
        errMsg = "Error: Not enough buffer pool size to run the query! Please reduce number of threads or increase shared memory pool size or reduce default page size and retry";
        std::cout << errMsg << std::endl;
        return 0;
    }
    backendCircularBufferSize = (conf->getShmSize() / conf->getPageSize() - 4 - 2 * numThreads);
    if (backendCircularBufferSize > 10) {
        backendCircularBufferSize = 10;
    }
    success = true;
    PDB_COUT << "backendCircularBufferSize is tuned to be " << backendCircularBufferSize
             << std::endl;
    return backendCircularBufferSize;
}

// to get iterators to scan a user set
std::vector<PageCircularBufferIteratorPtr> PipelineStage::getUserSetIterators(
    HermesExecutionServer* server, int numScanThreads, bool& success, std::string& errMsg) {

    // initialize the data proxy, scanner and set iterators
    PDBCommunicatorPtr communicatorToFrontend = make_shared<PDBCommunicator>();
    communicatorToFrontend->connectToInternetServer(
        logger, conf->getPort(), conf->getServerAddress(), errMsg);

    PDBLoggerPtr scannerLogger = make_shared<PDBLogger>("scanner.log");
    // getScanner
    int backendCircularBufferSize = getBackendCircularBufferSize(success, errMsg);
    PageScannerPtr scanner = make_shared<PageScanner>(communicatorToFrontend,
                                                      shm,
                                                      scannerLogger,
                                                      numScanThreads,
                                                      backendCircularBufferSize,
                                                      nodeId);

    std::vector<PageCircularBufferIteratorPtr> iterators;

    if (server->getFunctionality<HermesExecutionServer>().setCurPageScanner(scanner) == false) {
        success = false;
        errMsg = "Error: A job is already running!";
        std::cout << errMsg << std::endl;
        return iterators;
    }

    // get iterators
    iterators = scanner->getSetIterators(nodeId,
                                         jobStage->getSourceContext()->getDatabaseId(),
                                         jobStage->getSourceContext()->getTypeId(),
                                         jobStage->getSourceContext()->getSetId());

    // return iterators
    return iterators;
}


// to get iterators to scan a user set in a shared way so that each iterator gets all pages
void PipelineStage::feedSharedBuffers(HermesExecutionServer* server,
                                      std::vector<PageCircularBufferPtr>& sourceBuffers,
                                      int numPartitions,
                                      atomic_int& counter,
                                      PDBBuzzerPtr tempBuzzer,
                                      bool& success,
                                      std::string& errMsg) {
    // get scan iterators
    std::vector<PageCircularBufferIteratorPtr> scanIterators =
        getUserSetIterators(server, 1, success, errMsg);
    int numScanThreads = scanIterators.size();
    // start multiple thread to scan the set
    counter = 0;
    for (int i = 0; i < numScanThreads; i++) {
        // each threads get a page and put the page to each source buffer
        PDBWorkerPtr worker =
            server->getFunctionality<HermesExecutionServer>().getWorkers()->getWorker();
        // start threads
        PDBWorkPtr myWork = make_shared<GenericWork>([&, i](PDBBuzzerPtr callerBuzzer) {
            // setup an output page to store intermediate results and final output
            PageCircularBufferIteratorPtr iter = scanIterators.at(i);
            PDBPagePtr page = nullptr;
            while (iter->hasNext()) {
                page = iter->next();
                if (page != nullptr) {
                    for (int j = 0; j < numPartitions; j++) {
                        page->incRefCount();
                    }
                    for (int j = 0; j < numPartitions; j++) {
                        sourceBuffers[j]->addPageToTail(page);
                    }
                }
            }
            callerBuzzer->buzz(PDBAlarm::WorkAllDone, counter);
        });

        worker->execute(myWork, tempBuzzer);
    }
}

// to create a data proxy
DataProxyPtr PipelineStage::createProxy(int i,
                                        pthread_mutex_t connection_mutex,
                                        std::string& errMsg) {

    // create a data proxy
    std::string loggerName = std::string("PipelineStage_") + std::to_string(i);
    PDBLoggerPtr logger = make_shared<PDBLogger>(loggerName);
    pthread_mutex_lock(&connection_mutex);
    PDBCommunicatorPtr anotherCommunicatorToFrontend = make_shared<PDBCommunicator>();
    anotherCommunicatorToFrontend->connectToInternetServer(
        logger, conf->getPort(), conf->getServerAddress(), errMsg);
    pthread_mutex_unlock(&connection_mutex);
    DataProxyPtr proxy = make_shared<DataProxy>(nodeId, anotherCommunicatorToFrontend, shm, logger);
    return proxy;
}

// to execute the pipeline work defined in a TupleSetJobStage
// iterators can be empty if hash input is used
// combinerBuffers can be empty if no combining is required
void PipelineStage::executePipelineWork(int i,
                                        SetSpecifierPtr outputSet,
                                        std::vector<PageCircularBufferIteratorPtr>& iterators,
                                        PartitionedHashSetPtr hashSet,
                                        DataProxyPtr proxy,
                                        std::vector<PageCircularBufferPtr>& sinkBuffers,
                                        HermesExecutionServer* server,
                                        PartitionedHashSetPtr partitionedHashSetForSink,
                                        std::string& errMsg) {

#ifdef REUSE_CONNECTION_FOR_AGG_NO_COMBINER
    // connections
    std::vector<PDBCommunicatorPtr> connections;
    for (int j = 0; j < jobStage->getNumNodes(); j++) {

        // get the i-th address
        std::string address = this->jobStage->getIPAddress(j);
        PDB_COUT << "address = " << address << std::endl;

        // get the i-th port
        int port = this->jobStage->getPort(j);
        PDB_COUT << "port = " << port << std::endl;

        // get aggregate computation
        PDBCommunicatorPtr communicator = std::make_shared<PDBCommunicator>();
        communicator->connectToInternetServer(logger, port, address, errMsg);
        connections.push_back(communicator);
    }
#endif

    // seed the random number generator for each thread
    srand(time(NULL));


// setup an output page to store intermediate results and final output
#ifdef ENABLE_LARGE_GRAPH
    const UseTemporaryAllocationBlock tempBlock{256 * 1024 * 1024};
#else
    const UseTemporaryAllocationBlock tempBlock{32 * 1024 * 1024};
#endif

    Handle<ComputePlan> plan = this->jobStage->getComputePlan();
    plan->nullifyPlanPointer();
    Handle<ComputePlan> newPlan = deepCopyToCurrentAllocationBlock<ComputePlan>(plan);

    bool isHashPartitionedJoinProbing = false;
    Handle<Computation> computation = nullptr;
    std::vector<std::string> buildTheseTupleSets;
    jobStage->getTupleSetsToBuildPipeline(buildTheseTupleSets);
    std::string sourceSpecifier = jobStage->getSourceTupleSetSpecifier();
    if (buildTheseTupleSets[0] != sourceSpecifier) {
        std::string producerComputationName =
            newPlan->getProducingComputationName(buildTheseTupleSets[0]);
        computation = newPlan->getPlan()->getNode(producerComputationName).getComputationHandle();
        if (computation->getComputationType() == "JoinComp") {
            isHashPartitionedJoinProbing = true;
        }
    }
    if (isHashPartitionedJoinProbing == false) {
        std::string producerComputationName = newPlan->getProducingComputationName(sourceSpecifier);
        computation = newPlan->getPlan()->getNode(producerComputationName).getComputationHandle();
    }


    Handle<SetIdentifier> sourceContext = this->jobStage->getSourceContext();


    // handle four types of sources
    if ((this->jobStage->isLocalJoinSink())
       && (computation->getComputationType() == "ScanUserSet")) {
        Handle<ScanUserSet<Object>> scanner 
            = unsafeCast<ScanUserSet<Object>, Computation>(computation);
        scanner->setIterator(iterators.at(i));
        scanner->setProxy(proxy);
        if (scanner->getBatchSize() <= 0) {
            scanner->setBatchSize(batchSize);
        }
        std::string partitionComputationName = this->jobStage->getPartitionComputationSpecifier();
        std::string partitionLambdaName = this->jobStage->getPartitionLambdaName();
        GenericLambdaObjectPtr myPartitionLambda = newPlan->getPlan()->getNode(
            partitionComputationName).getLambda(
            partitionLambdaName);
        scanner->setPartitionLambda(myPartitionLambda);
        scanner->setFollowedByLocalJoin(true);
        scanner->setPartitionId(i);
        scanner->setNumPartitions(this->jobStage->getNumTotalPartitions());
        scanner->setNumNodes(this->jobStage->getNumNodes());

        getAllocator().setPolicy(AllocatorPolicy::noReuseAllocator);
    } else if ((sourceContext->getSetType() == UserSetType) &&
        (computation->getComputationType() != "JoinComp")) {
        // input is a user set
        Handle<ScanUserSet<Object>> scanner = nullptr;
        if (computation->getComputationType() == "ScanUserSet") {
            scanner = unsafeCast<ScanUserSet<Object>, Computation>(computation);
        } else if (computation->getComputationType() == "SelectionComp") {
            Handle<SelectionComp<Object, Object>> selection =
                unsafeCast<SelectionComp<Object, Object>, Computation>(computation);
            scanner = selection->getOutputSetScanner();
        } else if (computation->getComputationType() == "MultiSelectionComp") {
            Handle<MultiSelectionComp<Object, Object>> multiSelection =
                unsafeCast<MultiSelectionComp<Object, Object>, Computation>(computation);
            scanner = multiSelection->getOutputSetScanner();
        } else if (computation->getComputationType() == "ClusterAggregationComp") {
            Handle<ClusterAggregateComp<Object, Object, Object, Object>> aggregator =
                unsafeCast<ClusterAggregateComp<Object, Object, Object, Object>, Computation>(
                    computation);
            scanner = aggregator->getOutputSetScanner();
        } else if (computation->getComputationType() == "PartitionComp") {
            Handle<PartitionComp<Object, Object>> partitioner =
                unsafeCast<PartitionComp<Object, Object>, Computation>(computation);
            scanner = partitioner->getOutputSetScanner();
            int nodeId = this->jobStage->getNodeId();
            partitioner->setNodeId(nodeId);

        } else {
            std::cout << "Error: we can't support source computation type "
                      << computation->getComputationType() << std::endl;
            return;
        }

        if (scanner != nullptr) {
            scanner->setIterator(iterators.at(i));
            scanner->setProxy(proxy);
            if ((scanner->getBatchSize() <= 0) || (scanner->getBatchSize() > 1000)) {
                scanner->setBatchSize(batchSize);
            }
        }
    } else if ((sourceContext->getSetType() == UserSetType) &&
               (computation->getComputationType() == "JoinComp")) {

        Handle<JoinComp<Object, Object, Object>> join =
            unsafeCast<JoinComp<Object, Object, Object>, Computation>(computation);
        join->setIterator(iterators.at(i));
        join->setProxy(proxy);
        if (join->getBatchSize() == 0) {
            join->setBatchSize(1);
        }
        join->setPartitionId(i);
        join->setNumPartitions(this->jobStage->getNumTotalPartitions());
        join->setNumNodes(this->jobStage->getNumNodes());

    } else {
        // input are hash tables
        Handle<ClusterAggregateComp<Object, Object, Object, Object>> aggregator =
            unsafeCast<ClusterAggregateComp<Object, Object, Object, Object>, Computation>(
                computation);
        void* pagePointer = hashSet->getPage(i, true);
        if (pagePointer != nullptr) {
            aggregator->setHashTablePointer(hashSet->getPage(i, true));
        } else {
            return;
        }
    }

    // handle probing
    std::map<std::string, ComputeInfoPtr> info;
    if ((this->jobStage->isProbing() == true) && (this->jobStage->getHashSets() != nullptr)) {
        Handle<Map<String, String>> hashSetsToProbe = this->jobStage->getHashSets();
        for (PDBMapIterator<String, String> mapIter = hashSetsToProbe->begin();
             mapIter != hashSetsToProbe->end();
             ++mapIter) {
            bool probePartitionedHashMap = false;
            std::string key = (*mapIter).key;
            std::string hashSetName = (*mapIter).value;
            if (hashSetName.find(':') == 0) {
                probePartitionedHashMap = true;
                hashSetName = hashSetName.substr(1);  
            }
            AbstractHashSetPtr hashSet = server->getHashSet(hashSetName);
            if (hashSet == nullptr) {
                std::cout << "ERROR in pipeline execution: data not found in hash set "
                          << hashSetName << "!" << std::endl;
                return;
            }
            if (hashSet->getHashSetType() == "SharedHashSet") {
                SharedHashSetPtr sharedHashSet = std::dynamic_pointer_cast<SharedHashSet>(hashSet);
                info[key] = std::make_shared<JoinArg>(*newPlan, sharedHashSet->getPage(), nullptr);
            } else if (hashSet->getHashSetType() == "PartitionedHashSet") {
                PartitionedHashSetPtr partitionedHashSet =
                        std::dynamic_pointer_cast<PartitionedHashSet>(hashSet);
                if (!probePartitionedHashMap && !this->jobStage->isLocalJoinProbe()) {
                    info[key] = std::make_shared<JoinArg>(*newPlan, partitionedHashSet->getPage(i, true), nullptr);
                } else {
                    std::string joinComputationName =
                         newPlan->getProducingComputationName(key);
                    Handle<Computation> joinComputation =
                         newPlan->getPlan()->getNode(joinComputationName).getComputationHandle();
                    Handle<JoinComp<Object, Object, Object>> join = 
                         unsafeCast<JoinComp<Object, Object, Object>, Computation>(joinComputation);
                    join->setPartitionId(i);
                    join->setNumPartitions(this->jobStage->getNumTotalPartitions());
                    join->setNumNodes(this->jobStage->getNumNodes());
                    info[key] = std::make_shared<JoinArg>(*newPlan, nullptr, partitionedHashSet);
                    
                }
            }
        }
    }


    Handle<JoinComp<Object, Object, Object>> join = nullptr;
    std::string targetSpecifier = jobStage->getTargetComputationSpecifier();
    if (targetSpecifier.find("ClusterAggregationComp") != std::string::npos) {
        Handle<Computation> aggComputation =
            newPlan->getPlan()->getNode(targetSpecifier).getComputationHandle();
        Handle<AbstractAggregateComp> aggregate =
            unsafeCast<AbstractAggregateComp, Computation>(aggComputation);
        int numPartitionsInCluster = this->jobStage->getNumTotalPartitions();
        aggregate->setNumNodes(jobStage->getNumNodes());
        aggregate->setNumPartitions(numPartitionsInCluster);
        aggregate->setBatchSize(this->batchSize);
    } else if (targetSpecifier.find("JoinComp") != std::string::npos) {
        Handle<Computation> joinComputation =
            newPlan->getPlan()->getNode(targetSpecifier).getComputationHandle();
        join = unsafeCast<JoinComp<Object, Object, Object>, Computation>(joinComputation);
        join->setNumPartitions(this->jobStage->getNumTotalPartitions());
        join->setNumNodes(this->jobStage->getNumNodes());
    } else if (targetSpecifier.find("PartitionComp") != std::string::npos) {
        Handle<Computation> partitionComputation =
            newPlan->getPlan()->getNode(targetSpecifier).getComputationHandle();
        Handle<PartitionComp<Object, Object>> partitioner =
            unsafeCast<PartitionComp<Object, Object>, Computation>(partitionComputation);
        int numPartitionsInCluster = this->jobStage->getNumTotalPartitions();
        partitioner->setNumPartitions(numPartitionsInCluster);
        partitioner->setNumNodes(jobStage->getNumNodes());
        int nodeId = this->jobStage->getNodeId();
        partitioner->setNodeId(nodeId);
    }

#ifdef REUSE_CONNECTION_FOR_AGG_NO_COMBINER
    char* mem = nullptr;
    if ((this->jobStage->isRepartition() == true) && (this->jobStage->isCombining() == false) &&
        (join == nullptr)) {
        mem = (char*)malloc(conf->getNetShufflePageSize());
        if (mem == nullptr) {
            std::cout << "PipelineStage.cc: Failed to allocate memory with size=" << conf->getNetShufflePageSize() << std::endl;
            exit(1);
        }

    }
#endif


    newPlan->nullifyPlanPointer();
    PDBPagePtr output = nullptr;
    auto begin = std::chrono::high_resolution_clock::now();
    PipelinePtr curPipeline = newPlan->buildPipeline(
        i,
        buildTheseTupleSets,
        this->jobStage->getSourceTupleSetSpecifier(),
        this->jobStage->getTargetComputationSpecifier(),
        [&]() -> std::pair<void*, size_t> {
            size_t headerSize = DEFAULT_PAGE_HEADER_SIZE;
            if (this->jobStage->isLocalJoinSink()) {
                void * bytes = partitionedHashSetForSink->getPage(i);
                if (bytes == nullptr) {
                     std::cout << "Error: Insufficient memory in heap when allocating local join sink" << std::endl;
                     exit(1);
                }
                return std::make_pair(bytes, partitionedHashSetForSink->getPageSize());

            } else if ((this->jobStage->isBroadcasting() == false) &&
                (this->jobStage->isRepartition() == false) &&
                (sourceContext->getSetType() == UserSetType)) {
                proxy->addUserPage(outputSet->getDatabaseId(),
                                   outputSet->getTypeId(),
                                   outputSet->getSetId(),
                                   output);
 
                if (output == nullptr) {
                    std::cout << "Pipeline Error: insufficient memory in heap" << std::endl;
                    return std::make_pair(nullptr, 0);
                }
                return std::make_pair(output->getBytes(), output->getSize());


            } else if ((this->jobStage->isBroadcasting() == false) &&
                       (this->jobStage->isRepartition() == false) &&
                       (sourceContext->getSetType() != UserSetType)) {

                // TODO: move this to Pangea
                void* myPage = calloc(outputSet->getPageSize(), 1);
                if (myPage == nullptr) {
                    std::cout << "Pipeline Error: insufficient memory in heap" << std::endl;
                }
                return std::make_pair((char*)myPage + headerSize,
                                      outputSet->getPageSize() - headerSize);
            } else if ((this->jobStage->isBroadcasting() == true) ||
                       ((this->jobStage->isRepartition() == true) &&
                        (this->jobStage->isCombining() == false) && (join != nullptr))) {
                // TODO: move this to Pangea
                // join case
                void* myPage = calloc(conf->getBroadcastPageSize(), 1);
                if (myPage == nullptr) {
                    std::cout << "Pipeline Error: insufficient memory in heap" << std::endl;
                }
                return std::make_pair((char*)myPage + headerSize, conf->getNetBroadcastPageSize());
            } else {
                // TODO: move this to Pangea
                // aggregation case
                void* myPage = calloc(conf->getShufflePageSize(), 1);
                if (myPage == nullptr) {
                    std::cout << "Pipeline Error: insufficient memory in heap" << std::endl;
                }
                return std::make_pair((char*)myPage + headerSize, conf->getNetShufflePageSize());
            }

        },

        [&](void* page) {

            if ((this->jobStage->isBroadcasting() == false) &&
                (this->jobStage->isRepartition() == false) &&
                (sourceContext->getSetType() == UserSetType)) {
                if (output != nullptr) {
                    proxy->unpinUserPage(
                        nodeId, output->getDbID(), output->getTypeID(), output->getSetID(), output);
                }

            } else {
                if (!this->jobStage->isLocalJoinSink()) {
                    free((char*)page - DEFAULT_PAGE_HEADER_SIZE);
                }
            }
        },

        [&](void* page) {
            size_t headerSize = DEFAULT_PAGE_HEADER_SIZE;
            if (this->jobStage->isLocalJoinSink()) {
                Record<Object>* record = (Record<Object>*)page;
                if (record != nullptr) {
                    Handle<Object> object = record->getRootObject();
                    Handle<JoinMap<Object>> sinkMap = unsafeCast<JoinMap<Object>, Object>(object);  
                }
            } else if (this->jobStage->isBroadcasting() == true) {
                PDB_COUT << "to broadcast a page" << std::endl;
                // to handle a broadcast join
                // get the objects
                Record<Object>* record = (Record<Object>*)page;
                // broadcast the objects
                if (record != nullptr) {
                    Handle<Object> objectToSend = record->getRootObject();
                    if (objectToSend != nullptr) {
                        PDBPagePtr pageToBroadcast = std::make_shared<PDBPage>(
                            ((char*)page - headerSize),
                            0,
                            0,
                            0,
                            0,
                            0,
                            conf->getBroadcastPageSize(),
                            0,
                            0);
                        int numNodes = jobStage->getNumNodes();
                        int k;
                        NodeID myNodeId = jobStage->getNodeId();
                        for (k = 0; k < numNodes; k++) {
                            pageToBroadcast->incRefCount();
                        }
                        for (k = 0; k < numNodes; k++) {
                            if (k != myNodeId) {
                                PageCircularBufferPtr buffer = sinkBuffers[k];
                                buffer->addPageToTail(pageToBroadcast);
                            }
                        }
                        proxy->pinBytes(outputSet->getDatabaseId(),
                                        outputSet->getTypeId(),
                                        outputSet->getSetId(),
                                        record->numBytes(),
                                        record);
                        pageToBroadcast->decRefCount();
                        if (pageToBroadcast->getRefCount() == 0) {
                            pageToBroadcast->freeContent();
                        }
                    } else {
                        free((char*)page - headerSize);
                    }
                } else {
                    free((char*)page - headerSize);
                }
            } else if ((this->jobStage->isRepartition() == true) &&
                       (this->jobStage->isCombining() == false) && (join != nullptr)) {
                // to handle a hash partition join
                // get the objects
                Record<Object>* record = (Record<Object>*)page;
                // broadcast the objects
                if (record != nullptr) {
                    Handle<Object> objectToSend = record->getRootObject();
                    if (objectToSend != nullptr) {
                        PDBPagePtr pageToSend = std::make_shared<PDBPage>(
                            ((char*)page - headerSize),
                            0,
                            0,
                            0,
                            0,
                            0,
                            conf->getBroadcastPageSize()-sizeof(size_t),
                            0,
                            0);
                        int numNodes = jobStage->getNumNodes();
                        int k;
                        for (k = 0; k < numNodes; k++) {
                            pageToSend->incRefCount();
                        }
                        for (k = 0; k < numNodes; k++) {
                            PageCircularBufferPtr buffer = sinkBuffers[k];
                            buffer->addPageToTail(pageToSend);
                        }
                    } else {
                        free((char*)page - headerSize);
                    }
                } else {
                    free((char*)page - headerSize);
                }

            } else if ((this->jobStage->isRepartition() == true) &&
                       (this->jobStage->isCombining() == true)) {
                // to handle an aggregation
                PDBPagePtr output = make_shared<PDBPage>(
                    (char*)page - headerSize,
                    0,
                    0,
                    0,
                    0,
                    0,
                    conf->getShufflePageSize(),
                    0,
                    0);
                int numNodes = jobStage->getNumNodes();
                int k;
                for (k = 0; k < numNodes; k++) {
                    output->incRefCount();
                }
                for (k = 0; k < numNodes; k++) {
                    PageCircularBufferPtr buffer = sinkBuffers[k];
                    buffer->addPageToTail(output);
                }

            } else if ((this->jobStage->isRepartition() == true) &&
                       (this->jobStage->isCombining() == false) && (join == nullptr)) {
                // to handle aggregation without combining
                // to handle an aggregation
                Record<Vector<Handle<Vector<Handle<Object>>>>>* record =
                    (Record<Vector<Handle<Vector<Handle<Object>>>>>*)page;
                if (record != nullptr) {
                    Handle<Vector<Handle<Vector<Handle<Object>>>>> objectsToShuffle =
                        record->getRootObject();
                    int numNodes = jobStage->getNumNodes();
                    int k;
                    for (k = 0; k < numNodes; k++) {
                        Handle<Vector<Handle<Object>>> objectToShuffle = (*objectsToShuffle)[k];
#ifdef REUSE_CONNECTION_FOR_AGG_NO_COMBINER
                        Record<Vector<Handle<Object>>>* myRecord =
                            getRecord(objectToShuffle, mem, conf->getNetShufflePageSize());
                        if (objectToShuffle != nullptr) {
                            // to shuffle data
                            sendData(connections[k],
                                     myRecord,
                                     myRecord->numBytes(),
                                     jobStage->getSinkContext()->getDatabase(),
                                     jobStage->getSinkContext()->getSetName(),
                                     errMsg);
                        }
#else
                        if (objectToShuffle != nullptr) {
                            // to shuffle data
                            // get the i-th address
                            std::string address = this->jobStage->getIPAddress(k);
                            PDB_COUT << "address = " << address << std::endl;

                            // get the i-th port
                            int port = this->jobStage->getPort(k);
                            PDB_COUT << "port = " << port << std::endl;
                            this->storeShuffleData(objectToShuffle,
                                                   this->jobStage->getSinkContext()->getDatabase(),
                                                   this->jobStage->getSinkContext()->getSetName(),
                                                   address,
                                                   port,
                                                   errMsg);
                        }
#endif
                    }
                }
                free((char*)page - headerSize);

            } else {
                if (sourceContext->getSetType() == UserSetType) {
                    PDBPagePtr pageToUnpin = make_shared<PDBPage>((char*)page -
                             DEFAULT_PAGE_HEADER_SIZE, 0, 0);
                    proxy->unpinUserPage(nodeId, pageToUnpin->getDbID(), pageToUnpin->getTypeID(), pageToUnpin->getSetID(), pageToUnpin);
                } else {
                    // to handle a vector sink
                    // PDBPagePtr output = nullptr;
                    proxy->addUserPage(outputSet->getDatabaseId(),
                                       outputSet->getTypeId(),
                                       outputSet->getSetId(),
                                       output);
                    memcpy(output->getBytes(), page, output->getSize());
                    proxy->unpinUserPage(
                        nodeId, output->getDbID(), output->getTypeID(), output->getSetID(), output);
                    free((char*)page - headerSize);
                }
            }
        },

        info);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "BuildPipeline Duration for : " << this->jobStage->getSourceTupleSetSpecifier()
              << "-" << this->jobStage->getTargetComputationSpecifier()
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
    std::cout << i<<": Running Pipeline\n";
    curPipeline->id = i;
    curPipeline->run();
    curPipeline = nullptr;
    newPlan->nullifyPlanPointer();
    getAllocator().setPolicy(AllocatorPolicy::defaultAllocator);
#ifdef REUSE_CONNECTION_FOR_AGG_NO_COMBINER
    makeObjectAllocatorBlock(4 * 1024 * 1024, true);
    for (int j = 0; j < jobStage->getNumNodes(); j++) {
        sendData(connections[j],
                 nullptr,
                 DEFAULT_NET_PAGE_SIZE,
                 jobStage->getSinkContext()->getDatabase(),
                 jobStage->getSinkContext()->getSetName(),
                 errMsg);
    }
    if (mem != nullptr) {
        free(mem);
    }
#endif
}


void PipelineStage::runPipeline(HermesExecutionServer* server) {

    std::vector<PageCircularBufferPtr> sinkBuffers;
    SetSpecifierPtr outputSet =
        make_shared<SetSpecifier>(jobStage->getSinkContext()->getDatabase(),
                                  jobStage->getSinkContext()->getSetName(),
                                  jobStage->getSinkContext()->getDatabaseId(),
                                  jobStage->getSinkContext()->getTypeId(),
                                  jobStage->getSinkContext()->getSetId(),
                                  jobStage->getSinkContext()->getPageSize());
    runPipeline(server, sinkBuffers, outputSet);
}


// combinerBuffers can be empty if the pipeline doesn't need combining
void PipelineStage::runPipeline(HermesExecutionServer* server,
                                std::vector<PageCircularBufferPtr> sinkBuffers,
                                SetSpecifierPtr outputSet) {
#ifdef ENABLE_LARGE_GRAPH
    UseTemporaryAllocationBlock tempBlock{256 * 1024 * 1024};
#else
    UseTemporaryAllocationBlock tempBlock{32 * 1024 * 1024};
#endif
    bool success;
    std::string errMsg;
    int numPartitions = 0;
    atomic_int sourceCounter;
    sourceCounter = 0;
    PDBBuzzerPtr sourceBuzzer = nullptr;
    std::vector<PageCircularBufferPtr> sourceBuffers;
    // get user set iterators
    std::vector<PageCircularBufferIteratorPtr> iterators;
    PartitionedHashSetPtr hashSet = nullptr;
    Handle<SetIdentifier> sourceContext = this->jobStage->getSourceContext();

    // to get computations
    Handle<ComputePlan> plan = this->jobStage->getComputePlan();
    plan->nullifyPlanPointer();
    PDB_COUT << "to deep copy ComputePlan object" << std::endl;
    Handle<ComputePlan> newPlan = deepCopyToCurrentAllocationBlock<ComputePlan>(plan);
    bool isHashPartitionedJoinProbing = false;
    Handle<Computation> computation = nullptr;
    std::vector<std::string> buildTheseTupleSets;
    jobStage->getTupleSetsToBuildPipeline(buildTheseTupleSets);
    PDB_COUT << "buildTheseTupleSets[0]=" << buildTheseTupleSets[0] << std::endl;
    std::string sourceSpecifier = jobStage->getSourceTupleSetSpecifier();
    PDB_COUT << "Source tupleset name=" << sourceSpecifier << std::endl;
    if (buildTheseTupleSets[0] != sourceSpecifier) {
        std::string producerComputationName =
            newPlan->getProducingComputationName(buildTheseTupleSets[0]);
        PDB_COUT << "Producer computation name=" << producerComputationName << std::endl;
        computation = newPlan->getPlan()->getNode(producerComputationName).getComputationHandle();
        if (computation->getComputationType() == "JoinComp") {
            isHashPartitionedJoinProbing = true;
        }
    }
    if (isHashPartitionedJoinProbing == false) {
        std::string producerComputationName = newPlan->getProducingComputationName(sourceSpecifier);
        PDB_COUT << "Producer computation name=" << producerComputationName << std::endl;
        computation = newPlan->getPlan()->getNode(producerComputationName).getComputationHandle();
    }

    if (((this->jobStage->isLocalJoinSink()) 
       && (computation->getComputationType() == "ScanUserSet")) 
       || ((sourceContext->getSetType() == UserSetType) 
       && (computation->getComputationType() == "JoinComp"))) {
        int sourceBufferSize = 24; //efficient use of memory
        int numPartitionsInCluster = this->jobStage->getNumTotalPartitions();
        int numNodes = this->jobStage->getNumNodes();
        numPartitions = numPartitionsInCluster / numNodes;
        for (int i = 0; i < numPartitions; i++) {
            PDBLoggerPtr myLogger =
                make_shared<PDBLogger>(std::string("scanPartitionedSource-") + std::to_string(i));
            PageCircularBufferPtr buffer =
                make_shared<PageCircularBuffer>(sourceBufferSize, myLogger);
            sourceBuffers.push_back(buffer);
            PageCircularBufferIteratorPtr iter =
                make_shared<PageCircularBufferIterator>(i, buffer, myLogger);
            iterators.push_back(iter);
        }
    } else if ((sourceContext->getSetType() == UserSetType) &&
        (computation->getComputationType() != "JoinComp")) {
        iterators = getUserSetIterators(server, numThreads, success, errMsg);
    } else {
        std::string hashSetName = sourceContext->getDatabase() + ":" + sourceContext->getSetName();
        AbstractHashSetPtr abstractHashSet = server->getHashSet(hashSetName);
        hashSet = std::dynamic_pointer_cast<PartitionedHashSet>(abstractHashSet);
        numThreads = hashSet->getNumPages();
    }


    int numSourceThreads = numThreads;
    if (numPartitions > 0) {
        numSourceThreads = numPartitions;
    }

    //handling local join sink
    PartitionedHashSetPtr partitionedSetForSink = nullptr;
    if (this->jobStage->isLocalJoinSink()) {

        std::string hashSetName = this->jobStage->getSinkContext()->getDatabase()+":"+this->jobStage->getSinkContext()->getSetName();
        size_t hashSetSize = ((size_t)(this->jobStage->getSourceContext()->getNumPages())) * this->jobStage->getSourceContext()->getPageSize() * 3.0 / numSourceThreads;
        if (hashSetSize != DEFAULT_HASH_SET_SIZE)
            hashSetSize = DEFAULT_HASH_SET_SIZE; 
        partitionedSetForSink = make_shared<PartitionedHashSet>(hashSetName, hashSetSize);
        server->addHashSet(hashSetName, partitionedSetForSink);
        for (int i = 0; i < numSourceThreads; i++) {
            if (partitionedSetForSink->addPage() == nullptr) {
                std::cout << "Fatal Error: insufficient memory when allocating the " << i << "-th map" << std::endl;
                exit(1);
            }
        }
    }



    // initialize mutextes
    pthread_mutex_t connection_mutex;
    pthread_mutex_init(&connection_mutex, nullptr);
    
    atomic_int pipelineCounter;
    pipelineCounter = 0;

    // create a buzzer and counter
    PDBBuzzerPtr tempBuzzer =
        make_shared<PDBBuzzer>([](PDBAlarm myAlarm, atomic_int& counter) { 
            counter++; 
       }
       );


    for (int i = 0; i < numSourceThreads; i++) {
        PDBWorkerPtr worker =
            server->getFunctionality<HermesExecutionServer>().getWorkers()->getWorker();
        // TODO: start threads
        PDBWorkPtr myWork = make_shared<GenericWork>([&, i](PDBBuzzerPtr callerBuzzer) {

            std::string out = getAllocator().printInactiveBlocks();
            logger->warn(out);
#ifdef PROFILING
            std::cout << "print inactive blocks before running pipeline in this worker:"
                      << std::endl;
            std::cout << out << std::endl;
#endif
            // create a data proxy
            DataProxyPtr proxy = createProxy(i, connection_mutex, errMsg);

            // set allocator policy
            getAllocator().setPolicy(jobStage->getAllocatorPolicy());

            // setup an output page to store intermediate results and final output
            executePipelineWork(
                i, outputSet, iterators, hashSet, proxy, sinkBuffers, server, partitionedSetForSink, errMsg);

            // restore allocator policy
            getAllocator().setPolicy(AllocatorPolicy::defaultAllocator);
#ifdef PROFILING
            out = getAllocator().printInactiveBlocks();
            std::cout << "print inactive blocks after running pipeline in this worker:"
                      << std::endl;
            std::cout << out << std::endl;
#endif
            callerBuzzer->buzz(PDBAlarm::WorkAllDone, pipelineCounter);

        }

                                                     );
        worker->execute(myWork, tempBuzzer);
    }


    if (((this->jobStage->isLocalJoinSink())
       && (computation->getComputationType() == "ScanUserSet"))
       || ((sourceContext->getSetType() == UserSetType)
       && (computation->getComputationType() == "JoinComp"))) {
        // start the scanning thread
        sourceCounter = 0;
        sourceBuzzer = make_shared<PDBBuzzer>([](PDBAlarm myAlarm, atomic_int& sourceCounter) {
            sourceCounter++;
        });
        // get scan iterators
        std::vector<PageCircularBufferIteratorPtr> scanIterators =
            getUserSetIterators(server, 1, success, errMsg);
        int numScanThreads = scanIterators.size();
        // start multiple thread to scan the set
        for (int i = 0; i < numScanThreads; i++) {
            // each threads get a page and put the page to each source buffer
            PDBWorkerPtr worker =
                server->getFunctionality<HermesExecutionServer>().getWorkers()->getWorker();
            // start threads
            PDBWorkPtr myWork = make_shared<GenericWork>([&, i](PDBBuzzerPtr callerBuzzer) {
                // setup an output page to store intermediate results and final output
                const UseTemporaryAllocationBlock tempBlock{4 * 1024 * 1024};
                PageCircularBufferIteratorPtr iter = scanIterators[i];
                PDBPagePtr page = nullptr;
                while (iter->hasNext()) {
                    page = iter->next();
                    if (page != nullptr) {
                        for (int j = 0; j < numPartitions; j++) {
                            page->incRefCount();
                        }
                        for (int j = 0; j < numPartitions; j++) {
                            sourceBuffers[j]->addPageToTail(page);
                        }
                    } else {
                        sched_yield();
                    }
                }
                
                callerBuzzer->buzz(PDBAlarm::WorkAllDone, sourceCounter);
            });

            worker->execute(myWork, sourceBuzzer);
        }  // for

        while (sourceCounter < 1 ) {
            sourceBuzzer->wait();
        }
        sourceCounter = 0;

        for (int i = 0; i < numPartitions; i++) {
            PageCircularBufferPtr buffer = sourceBuffers[i];
            buffer->close();
        }

        while (pipelineCounter < numSourceThreads) {
            sched_yield();
            tempBuzzer->wait();
        }

    } else {

        while (pipelineCounter < numSourceThreads) {
            sched_yield();
            tempBuzzer->wait();
        }

    }

    pipelineCounter = 0;

    pthread_mutex_destroy(&connection_mutex);


    if (server->getFunctionality<HermesExecutionServer>().setCurPageScanner(nullptr) == false) {
        success = false;
        errMsg = "Error: No job is running!";
        std::cout << errMsg << std::endl;
        return;
    }

    return;
}


// below method will run the combiner
void PipelineStage::runPipelineWithShuffleSink(HermesExecutionServer* server) {
    bool success;
    std::string errMsg;

    int numNodes = jobStage->getNumNodes();

 
#ifdef AUTO_TUNING
    size_t memSize = jobStage->getTotalMemoryOnThisNode();
    size_t sharedMemPoolSize = conf->getShmSize();
    size_t hashSetSize = server->getHashSetsSize();

#ifndef USE_VALGRIND
    tunedHashPageSize =
        (double)((memSize * ((size_t)(1024)) - sharedMemPoolSize - server->getHashSetsSize())/(size_t)(numNodes)) *
        (0.8);
#else
    tunedHashPageSize =
        (double)((memSize * ((size_t)(1024)) - sharedMemPoolSize - server->getHashSetsSize())/(size_t)(numNodes)) *
        (0.5);
#endif
    if (memSize * ((size_t)(1024)) <
        sharedMemPoolSize + (size_t)512 * (size_t)1024 * (size_t)1024) {
        std::cout << "WARNING: Auto tuning can not work for this case, we use default value"
                  << std::endl;
        tunedHashPageSize = conf->getHashPageSize();
    }
    if (tunedHashPageSize > (size_t)(1024)*(size_t)(1024)*(size_t)(1024)) {
            std::cout << "The auto tuned hash page size is bigger than 1G: " << tunedHashPageSize << std::endl;
            tunedHashPageSize = (size_t)(1024)*(size_t)(1024)*(size_t)(1024);
        }
#endif


    // each queue has multiple producers and one consumer
    int combinerBufferSize = numThreads;
    if (combinerBufferSize > 12) {
        combinerBufferSize = 12;
    }
    std::vector<PageCircularBufferPtr> combinerBuffers;
    std::vector<PageCircularBufferIteratorPtr> combinerIters;

    pthread_mutex_t connection_mutex;
    pthread_mutex_init(&connection_mutex, nullptr);

    // create a buzzer and counter
    PDBBuzzerPtr combinerBuzzer =
        make_shared<PDBBuzzer>([&](PDBAlarm myAlarm, atomic_int& combinerCounter) {
            combinerCounter++;
            PDB_COUT << "combinerCounter = " << combinerCounter << std::endl;
        });
    PDB_COUT << "to run combiner with " << numNodes << " threads." << std::endl;
    atomic_int combinerCounter;
    combinerCounter = 0;

    size_t combinerPageSize = conf->getShufflePageSize();
    int i;
    for (i = 0; i < numNodes; i++) {
        PageCircularBufferPtr buffer = make_shared<PageCircularBuffer>(combinerBufferSize, logger);
        combinerBuffers.push_back(buffer);
        PageCircularBufferIteratorPtr iter =
            make_shared<PageCircularBufferIterator>(i, buffer, logger);
        combinerIters.push_back(iter);
        PDBWorkerPtr worker =
            server->getFunctionality<HermesExecutionServer>().getWorkers()->getWorker();
        // start threads
        PDBWorkPtr myWork = make_shared<GenericWork>([&, i](PDBBuzzerPtr callerBuzzer) {

            std::string out = getAllocator().printInactiveBlocks();
            logger->warn(out);
#ifdef PROFILING
            std::cout << "inactive blocks before running combiner in this worker:" << std::endl;
            std::cout << out << std::endl;
#endif

            std::string errMsg;

            // create data proxy
            DataProxyPtr proxy = createProxy(i, connection_mutex, errMsg);

            std::string address = "";
            int port = 0;
            int numNodesToCollect = this->jobStage->getNumNodesToCollect();
            if (this->jobStage->isCollectAsMap() == false) {

                // get the i-th address
                address = this->jobStage->getIPAddress(i);

                // get the i-th port
                port = this->jobStage->getPort(i);

            } else {

                // get the 1-st address
                address = this->jobStage->getIPAddress(i % numNodesToCollect);

                // get the 1-st port
                port = this->jobStage->getPort(i % numNodesToCollect);
            }
            // get aggregate computation
#ifdef ENABLE_LARGE_GRAPH
            const UseTemporaryAllocationBlock tempBlock{512 * 1024 * 1024};
#else
            const UseTemporaryAllocationBlock tempBlock{32 * 1024 * 1024};
#endif
            Handle<ComputePlan> plan = this->jobStage->getComputePlan();
            plan->nullifyPlanPointer();
            Handle<ComputePlan> newPlan = deepCopyToCurrentAllocationBlock<ComputePlan>(plan);
            std::string targetSpecifier = jobStage->getTargetComputationSpecifier();
            Handle<Computation> computation =
                newPlan->getPlan()->getNode(targetSpecifier).getComputationHandle();
            Handle<AbstractAggregateComp> aggregate =
                unsafeCast<AbstractAggregateComp, Computation>(computation);
            Handle<Vector<HashPartitionID>> partitions = this->jobStage->getNumPartitions(i);
            std::vector<HashPartitionID> stdPartitions;
            int numPartitionsOnTheNode = partitions->size();
            for (int m = 0; m < numPartitionsOnTheNode; m++) {
                stdPartitions.push_back((*partitions)[m]);
            }
            // get combiner processor
            SimpleSingleTableQueryProcessorPtr combinerProcessor =
                aggregate->getCombinerProcessor(stdPartitions);
            size_t myCombinerPageSize = combinerPageSize;
            void* combinerPage = (void*)calloc(myCombinerPageSize, sizeof(char));
            if (combinerPage == nullptr) {
                std::cout << "Fatal Error: insufficient memory can be allocated from memory, please tune the shufflePageSize in netsdb/src/conf/headers/Configuration.h"
                          << std::endl;
                exit(-1);
            }
            combinerProcessor->loadOutputPage(combinerPage, myCombinerPageSize);

            PageCircularBufferIteratorPtr myIter = combinerIters[i];
            int numPages = 0;
            while (myIter->hasNext()) {
                PDBPagePtr page = myIter->next();
                if (page != nullptr) {
                    // to load input page
                    numPages++;
                    combinerProcessor->loadInputPage(page->getBytes());
                    while (combinerProcessor->fillNextOutputPage()) {
                        // send out the output page
                        Record<Vector<Handle<Object>>>* record =
                            (Record<Vector<Handle<Object>>>*)combinerPage;
#ifndef ENABLE_COMPRESSION
                        this->storeShuffleData(record->getRootObject(),
                                               this->jobStage->getSinkContext()->getDatabase(),
                                               this->jobStage->getSinkContext()->getSetName(),
                                               address,
                                               port,
                                               errMsg);
#else
                        char* compressedBytes =
                            new char[snappy::MaxCompressedLength(record->numBytes())];
                        if (compressedBytes == nullptr) {
                            std::cout << "PipelineStage.cc: failed to allocate memory" << std::endl;
                            exit(1);
                        }
                        size_t compressedSize;
                        snappy::RawCompress(
                            (char*)record, record->numBytes(), compressedBytes, &compressedSize);
                        std::cout << "size before compression is " << record->numBytes()
                                  << " and size after compression is " << compressedSize
                                  << std::endl;
                        this->storeCompressedShuffleData(
                            compressedBytes,
                            compressedSize,
                            this->jobStage->getSinkContext()->getDatabase(),
                            this->jobStage->getSinkContext()->getSetName(),
                            address,
                            port,
                            errMsg);
                        delete[] compressedBytes;

#endif


                        // free the output page
                        combinerProcessor->clearOutputPage();
                        free(combinerPage);
                        // allocate a new page
                        combinerPage = (void*)malloc(myCombinerPageSize * sizeof(char));
                        if (combinerPage == nullptr) {
                            std::cout
                                << "Fatal Error: insufficient memory can be allocated from memory"
                                << std::endl;
                            exit(-1);
                        }
                        // load the new page as output vector
                        combinerProcessor->loadOutputPage(combinerPage, myCombinerPageSize);
                    }
                    // unpin the input page
                    // combinerProcessor->clearInputPage();
                    page->decRefCount();
                    if (page->getRefCount() == 0) {
                        page->freeContent();
                    }
                }
            }
            combinerProcessor->finalize();
            combinerProcessor->fillNextOutputPage();
            // send the output page
            Record<Vector<Handle<Object>>>* record = (Record<Vector<Handle<Object>>>*)combinerPage;
#ifndef ENABLE_COMPRESSION
            this->storeShuffleData(record->getRootObject(),
                                   this->jobStage->getSinkContext()->getDatabase(),
                                   this->jobStage->getSinkContext()->getSetName(),
                                   address,
                                   port,
                                   errMsg);
#else
            char* compressedBytes = new char[snappy::MaxCompressedLength(record->numBytes())];
            if (compressedBytes == nullptr) {
                std::cout << "PipelineStage.cc: failed to allocate memory" << std::endl;
                exit(1);
            }
            size_t compressedSize;
            snappy::RawCompress(
                (char*)record, record->numBytes(), compressedBytes, &compressedSize);
            this->storeCompressedShuffleData(compressedBytes,
                                             compressedSize,
                                             this->jobStage->getSinkContext()->getDatabase(),
                                             this->jobStage->getSinkContext()->getSetName(),
                                             address,
                                             port,
                                             errMsg);
            delete[] compressedBytes;

#endif

            // free the output page
            combinerProcessor->clearOutputPage();
            free(combinerPage);
            getAllocator().setPolicy(defaultAllocator);
#ifdef PROFILING
            out = getAllocator().printInactiveBlocks();
            std::cout << "inactive blocks after running combiner in this worker:" << std::endl;
            std::cout << out << std::endl;
#endif
            callerBuzzer->buzz(PDBAlarm::WorkAllDone, combinerCounter);
        }

                                                     );
        worker->execute(myWork, combinerBuzzer);
    }
    SetSpecifierPtr outputSet =
        make_shared<SetSpecifier>(jobStage->getCombinerContext()->getDatabase(),
                                  jobStage->getCombinerContext()->getSetName(),
                                  jobStage->getCombinerContext()->getDatabaseId(),
                                  jobStage->getCombinerContext()->getTypeId(),
                                  jobStage->getCombinerContext()->getSetId(),
                                  jobStage->getCombinerContext()->getPageSize());
    runPipeline(server, combinerBuffers, outputSet);


    int k;
    for (k = 0; k < numNodes; k++) {
        PageCircularBufferPtr buffer = combinerBuffers[k];
        buffer->close();
    }

    while (combinerCounter < numNodes) {
        combinerBuzzer->wait();
    }

    combinerCounter = 0;
    return;
}

// below method will run broadcasting
void PipelineStage::runPipelineWithBroadcastSink(HermesExecutionServer* server) {

    bool success;
    std::string errMsg;

    int numNodes = jobStage->getNumNodes();

    // each queue has multiple producers and one consumer
    int shuffleBufferSize = numThreads;
    if (shuffleBufferSize > 12) {
        shuffleBufferSize = 12;
    }
    PDB_COUT << "shuffleBufferSize=" << shuffleBufferSize << std::endl;
    std::vector<PageCircularBufferPtr> shuffleBuffers;
    std::vector<PageCircularBufferIteratorPtr> shuffleIters;

    pthread_mutex_t connection_mutex;
    pthread_mutex_init(&connection_mutex, nullptr);

    // create a buzzer and counter
    PDBBuzzerPtr shuffleBuzzer = make_shared<PDBBuzzer>([&](PDBAlarm myAlarm, atomic_int& shuffleCounter) {
        shuffleCounter++;
        PDB_COUT << "shuffleCounter = " << shuffleCounter << std::endl;
    });
    PDB_COUT << "to run shuffle with " << numNodes << " threads." << std::endl;
    atomic_int shuffleCounter;
    shuffleCounter = 0;

    int i;
    NodeID myNodeId = jobStage->getNodeId();
    for (i = 0; i < numNodes; i++) {
        PageCircularBufferPtr buffer = make_shared<PageCircularBuffer>(shuffleBufferSize, logger);
        shuffleBuffers.push_back(buffer);
        PageCircularBufferIteratorPtr iter =
            make_shared<PageCircularBufferIterator>(i, buffer, logger);
        shuffleIters.push_back(iter);
        PDBWorkerPtr worker =
            server->getFunctionality<HermesExecutionServer>().getWorkers()->getWorker();
        PDB_COUT << "to run the " << i << "-th broadcasting work..." << std::endl;
        // start threads
        PDBWorkPtr myWork = make_shared<GenericWork>([&, i](PDBBuzzerPtr callerBuzzer) {
            if (i == myNodeId) {
                callerBuzzer->buzz(PDBAlarm::WorkAllDone, shuffleCounter);
                return;
            }
            UseTemporaryAllocationBlock tempBlock{2 * 1024 * 1024};
            std::string out = getAllocator().printInactiveBlocks();
            logger->warn(out);
#ifdef PROFILING
            std::cout << "inactive blocks before sending data in this worker:" << std::endl;
            std::cout << out << std::endl;
#endif

            std::string errMsg;

            // get the i-th address
            std::string address = this->jobStage->getIPAddress(i);

            // get the i-th port
            int port = this->jobStage->getPort(i);

            // get aggregate computation

            PDBCommunicatorPtr communicator = std::make_shared<PDBCommunicator>();
            communicator->connectToInternetServer(logger, port, address, errMsg);

            PageCircularBufferIteratorPtr myIter = shuffleIters[i];
            int numPages = 0;
            while (myIter->hasNext()) {
                PDBPagePtr page = myIter->next();
                if (page != nullptr) {
                    // to load input page
                    numPages++;
                    // send out the page
                    Record<Object>* myRecord = (Record<Object>*)(page->getBytes());
                    sendData(communicator,
                             (char*)myRecord,
                             myRecord->numBytes(),
                             jobStage->getSinkContext()->getDatabase(),
                             jobStage->getSinkContext()->getSetName(),
                             errMsg);
                    // unpin the input page
                    page->decRefCount();
                    if (page->getRefCount() == 0) {
                        page->freeContent();
                    }
                }
            }
            sendData(communicator,
                     nullptr,
                     DEFAULT_NET_PAGE_SIZE,
                     jobStage->getSinkContext()->getDatabase(),
                     jobStage->getSinkContext()->getSetName(),
                     errMsg);
#ifdef PROFILING
            out = getAllocator().printInactiveBlocks();
            std::cout << "inactive blocks after sending data in this worker:" << std::endl;
            std::cout << out << std::endl;
#endif
            callerBuzzer->buzz(PDBAlarm::WorkAllDone, shuffleCounter);
        }

                                                     );
        worker->execute(myWork, shuffleBuzzer);
    }
    SetSpecifierPtr outputSet =
        make_shared<SetSpecifier>(jobStage->getSinkContext()->getDatabase(),
                                  jobStage->getSinkContext()->getSetName(),
                                  jobStage->getSinkContext()->getDatabaseId(),
                                  jobStage->getSinkContext()->getTypeId(),
                                  jobStage->getSinkContext()->getSetId(),
                                  jobStage->getSinkContext()->getPageSize());
    runPipeline(server, shuffleBuffers, outputSet);


    int k;
    for (k = 0; k < numNodes; k++) {
        PageCircularBufferPtr buffer = shuffleBuffers[k];
        buffer->close();
    }

    while (shuffleCounter < numNodes) {
        shuffleBuzzer->wait();
    }

    shuffleCounter = 0;
    return;
}

// below method will run hash partitioning
void PipelineStage::runPipelineWithHashPartitionSink(HermesExecutionServer* server) {

    bool success;
    std::string errMsg;


    // each queue has multiple producers and one consumer
    int shuffleBufferSize = 2*numThreads;
    std::vector<PageCircularBufferPtr> shuffleBuffers;
    std::vector<PageCircularBufferIteratorPtr> shuffleIters;

    atomic_int shuffleCounter;
    shuffleCounter = 0;
    // create a buzzer and counter
    PDBBuzzerPtr shuffleBuzzer = make_shared<PDBBuzzer>([](PDBAlarm myAlarm, atomic_int& counter) {
        counter++;
    });

    
    NodeID myNodeId = jobStage->getNodeId();
    int numNodes = jobStage->getNumNodes();
    int numThreadsForLocalStore = 1;

    for (int i = 0; i < numNodes; i++) {
        PageCircularBufferPtr buffer = make_shared<PageCircularBuffer>(shuffleBufferSize, logger);
        shuffleBuffers.push_back(buffer);
        PageCircularBufferIteratorPtr iter =
            make_shared<PageCircularBufferIterator>(i, buffer, logger);
        shuffleIters.push_back(iter);
        if (i == myNodeId) {
            for (int j = 0; j < numThreadsForLocalStore; j++) {
                PDBWorkerPtr worker =
                    server->getFunctionality<HermesExecutionServer>().getWorkers()->getWorker();
                // start thread
                PDBWorkPtr myWork = make_shared<HashPartitionWork>(i, iter, this, shuffleCounter);
                worker->execute(myWork, shuffleBuzzer);
            }

        } else {
            PDBWorkerPtr worker =
                server->getFunctionality<HermesExecutionServer>().getWorkers()->getWorker();
            // start thread
            PDBWorkPtr myWork = make_shared<HashPartitionWork>(i, iter, this, shuffleCounter); 
            worker->execute(myWork, shuffleBuzzer);
        }

    }
    SetSpecifierPtr outputSet =
        make_shared<SetSpecifier>(jobStage->getSinkContext()->getDatabase(),
                                  jobStage->getSinkContext()->getSetName(),
                                  jobStage->getSinkContext()->getDatabaseId(),
                                  jobStage->getSinkContext()->getTypeId(),
                                  jobStage->getSinkContext()->getSetId(),
                                  jobStage->getSinkContext()->getPageSize());
    runPipeline(server, shuffleBuffers, outputSet);
    int k;
    for (k = 0; k < numNodes; k++) {
        PageCircularBufferPtr buffer = shuffleBuffers[k];
        buffer->close();
    }


    while (shuffleCounter < numNodes - 1 + numThreadsForLocalStore) {
        shuffleBuzzer->wait();
    }

    shuffleCounter = 0;
    return;
}
}
#endif
