
#ifndef JOIN_COMP
#define JOIN_COMP

#include "Computation.h"
#include "JoinTests.h"
#include "ComputePlan.h"
#include "JoinTuple.h"
#include "JoinCompBase.h"
#include "MultiInputsBase.h"
#include "PageCircularBufferIterator.h"
#include "DataProxy.h"
#include "PDBPage.h"
#include "PartitionedHashSet.h"


namespace pdb {

// used to parameterize joins that are run as part of a pipeline
class JoinArg : public ComputeInfo {

public:
    // this is the compute plan that we are part of
    ComputePlan& plan;

    // the location of the hash table
    void* pageWhereHashTableIs;

    // the location of the partitioned hash table
    PartitionedHashSetPtr partitionedHashSet;


    JoinArg(ComputePlan& plan, void* pageWhereHashTableIs, 
            PartitionedHashSetPtr partitionedHashSet)
        : plan(plan), 
          pageWhereHashTableIs(pageWhereHashTableIs),
          partitionedHashSet(partitionedHashSet) {}

    ~JoinArg() {}
};



template <typename Out, typename In1, typename In2, typename... Rest>
class JoinComp : public JoinCompBase {

private:
    // JiaNote: this is used to pass to lambda tree to update pipeline information for each input
    MultiInputsBase* multiInputsBase = nullptr;

    // JiaNote: this is to specify the JoinType, by default we use broadcast join
    JoinType joinType = BroadcastJoin;

    // JiaNote: partition number in the cluster, used by hash partition join
    int numPartitions = 0;

    // JiaNote: number of nodes, used by hash partition join
    int numNodes = 0;

    // JiaNote: partitionId for JoinSource, used by hash partition join
    size_t myPartitionId;

    // JiaNote: the iterator for retrieving TupleSets from JoinMaps in pages
    // be careful here that we put PageCircularBufferIteratorPtr and DataProxyPtr in a pdb object.
    PageCircularBufferIteratorPtr iterator = nullptr;

    // JiaNote: the data proxy for accessing pages in frontend storage server.
    DataProxyPtr proxy = nullptr;

    // batch size
    int batchSize;


public:
    // set join type
    void setJoinType(JoinType joinType) {
        this->joinType = joinType;
    }

    // get join type
    JoinType getJoinType() {
        return this->joinType;
    }


    // set number of partitions  (used in hash partition join)
    void setNumPartitions(int numPartitions) {
        this->numPartitions = numPartitions;
    }

    // return my number of partitions  (used in hash partition join)
    int getNumPartitions() {
        return numPartitions;
    }

    // set number of nodes  (used in hash partition join)
    void setNumNodes(int numNodes) {
        this->numNodes = numNodes;
    }

    // return my number of nodes  (used in hash partition join)
    int getNumNodes() {
        return numNodes;
    }

    // set my partition id for obtaining JoinSource for one partition  (used in hash partition join)
    void setPartitionId(size_t myPartitionId) {
        this->myPartitionId = myPartitionId;
    }

    // return my partition id for obtaining JoinSource for one partition  (used in hash partition
    // join)
    int getPartitionId() {
        return this->myPartitionId;
    }

    // JiaNote: be careful here that we put PageCircularBufferIteratorPtr and DataProxyPtr in a pdb
    // object (used in hash partition join)
    void setIterator(PageCircularBufferIteratorPtr iterator) {
        this->iterator = iterator;
    }

    // to set proxy for communicating with frontend storage server (used in hash partition join)
    void setProxy(DataProxyPtr proxy) {
        this->proxy = proxy;
    }

    // to set chunk size for JoinSource (used in hash partition join)
    void setBatchSize(int batchSize) override {
        this->batchSize = batchSize;
    }

    // to get batch size for JoinSource (used in hash partition join)
    int getBatchSize() {
        return this->batchSize;
    }


    MultiInputsBase* getMultiInputsBase() {
        if (multiInputsBase == nullptr) {
            multiInputsBase = new MultiInputsBase();
        }
        return multiInputsBase;
    }

    virtual ~JoinComp() {
        if (multiInputsBase == nullptr) {
            delete (multiInputsBase);
        }
        this->iterator = nullptr;
        this->proxy = nullptr;
    }

    void setMultiInputsBaseToNull() {
        if (multiInputsBase == nullptr) {
            delete (multiInputsBase);
        }
        multiInputsBase = nullptr;
    }

    void analyzeInputSets(std::vector<std::string>& inputNames) {
        if (multiInputsBase == nullptr) {
            multiInputsBase = new MultiInputsBase();
        }
        // Step 1. setup all input names (the column name corresponding to input in tuple set)
        for (int i = 0; i < inputNames.size(); i++) {
            this->multiInputsBase->setNameForIthInput(i, inputNames[i]);
        }

        // Step 2. analyze selectionLambda to find all inputs in predicates
        Lambda<bool> selectionLambda = callGetSelection(*this);
        std::vector<std::string> inputsInPredicates =
            selectionLambda.getAllInputs(this->multiInputsBase);
        for (int i = 0; i < inputsInPredicates.size(); i++) {
            this->multiInputsBase->addInputNameToPredicates(inputsInPredicates[i]);
        }
        // Step 3. analyze projectionLambda to find all inputs in projection
        Lambda<Handle<Out>> projectionLambda = callGetProjection(*this);
        std::vector<std::string> inputsInProjection =
            projectionLambda.getAllInputs(this->multiInputsBase);
        for (int i = 0; i < inputsInProjection.size(); i++) {
            this->multiInputsBase->addInputNameToProjection(inputsInProjection[i]);
        }
    }


    // the computation returned by this method is called to see if a data item should be returned in
    // the output set
    virtual Lambda<bool> getSelection(Handle<In1> in1,
                                      Handle<In2> in2,
                                      Handle<Rest>... otherArgs) = 0;

    // the computation returned by this method is called to produce output tuples from this method
    virtual Lambda<Handle<Out>> getProjection(Handle<In1> in1,
                                              Handle<In2> in2,
                                              Handle<Rest>... otherArgs) = 0;

    // calls getProjection and getSelection to extract the lambdas
    void extractLambdas(std::map<std::string, GenericLambdaObjectPtr>& returnVal) override {
        int suffix = 0;
        Lambda<bool> selectionLambda = callGetSelection(*this);
        Lambda<Handle<Out>> projectionLambda = callGetProjection(*this);
        selectionLambda.toMap(returnVal, suffix);
        projectionLambda.toMap(returnVal, suffix);
    }

    //JiaNote: to populate lambdas for self learning
    void populateLambdas(long jobId, SelfLearningWrapperServer server) override {
        int suffix = 0;
        std::string compName = getComputationName();
        std::vector<std::string> computationInputTypes;
        int numInputs = getNumInputs();
        for (int i = 0; i < numInputs; i++) {
            std::string curInputType = getIthInputType(i);
            std::cout << curInputType << std::endl;
            computationInputTypes.push_back(curInputType);
        }
        Lambda<bool> selectionLambda = callGetSelection(*this);
        selectionLambda.toSelfLearningDatabase(jobId, compName, computationInputTypes, 
               server, suffix);
    }

    // return the output type
    std::string getOutputType() override {
        return getTypeName<Out>();
    }

    // count the number of inputs
    int getNumInputs() final {
        const int extras = sizeof...(Rest);
        return extras + 2;
    }

    template <typename First, typename... Others>
    typename std::enable_if<sizeof...(Others) == 0, std::string>::type getIthInputType(int i) {
        if (i == 0) {
            return getTypeName<First>();
        } else {
            std::cout << "Asked for an input type that didn't exist!";
            exit(1);
        }
    }

    // helper function to get a particular intput type
    template <typename First, typename... Others>
    typename std::enable_if<sizeof...(Others) != 0, std::string>::type getIthInputType(int i) {
        if (i == 0) {
            return getTypeName<First>();
        } else {
            return getIthInputType<Others...>(i - 1);
        }
    }

    // from the interface: get the i^th input type
    std::string getIthInputType(int i) final {
        return getIthInputType<In1, In2, Rest...>(i);
    }

    // we assume all ancesters of the join has only one input
    void getSources(std::vector<std::pair<std::string, std::string>>& sources) {
        int numSources = getNumInputs();
        Handle<Computation> curComputation = nullptr;
        for (int i = 0; i < numSources; i++) {
            curComputation = getIthInput(i);
            while (curComputation != nullptr){
                if(curComputation->getComputationType() == "ScanUserSet") {
                    break;
                }
                curComputation=curComputation->getIthInput(0);
            }
            if (curComputation != nullptr) {
                sources.push_back(std::pair<std::string, std::string>(curComputation->getDatabaseName(), curComputation->getSetName()));
            }
        }
        for (int i = 0; i < sources.size(); i++) {
            std::cout << i <<"-th source of join "<< sources[i].first << ":" << sources[i].second << std::endl;
        }
    }





    // JiaNote: TODO: encapsulate and reuse code for getting correctJoinTuple

    // JiaNote: this gets a sink merger
    SinkMergerPtr getSinkMerger(int partitionId,
                                TupleSpec& consumeMe,
                                TupleSpec& attsToOpOn,
                                TupleSpec& projection,
                                ComputePlan& plan) override {
        std :: cout << "to get sink merger for partitionId=" << partitionId << std :: endl;
        std :: cout << "consumeMe=" << consumeMe << std :: endl;
        std :: cout << "attsToOpOn=" << attsToOpOn << std :: endl;
        std :: cout << "projection=" << projection << std :: endl;
        // loop through each of the attributes that we are supposed to accept, and for each of them,
        // find the type
        std::vector<std::string> typeList;
        std::cout << "consumeMe.getSetName()=" << consumeMe.getSetName() << std::endl;
        AtomicComputationPtr producer =
            plan.getPlan()->getComputations().getProducingAtomicComputation(consumeMe.getSetName());
        for (auto& a : projection.getAtts()) {

            // find the identity of the producing computation
            std::pair<std::string, std::string> res =
                producer->findSource(a, plan.getPlan()->getComputations());

            // and find its type... in the first case, there is not a particular lambda that we need
            // to ask for
            if (res.second == "") {
                typeList.push_back(
                    "pdb::Handle<" +
                    plan.getPlan()->getNode(res.first).getComputation().getOutputType() + ">");
            } else {
                std::string myType =
                    plan.getPlan()->getNode(res.first).getLambda(res.second)->getOutputType();

                if (myType.find_first_of("pdb::Handle<") == 0) {
                    typeList.push_back(myType);
                } else {
                    typeList.push_back("pdb::Handle<" + myType + ">");
                }
            }
        }

         for (auto &aa : typeList) {
                 std :: cout << "Got type " << aa << "\n";
         }

        // now we get the correct join tuple, that will allow us to pack tuples from the join in a
        // hash table
        std::vector<int> whereEveryoneGoes;
        JoinTuplePtr correctJoinTuple =
            findCorrectJoinTuple<In1, In2, Rest...>(typeList, whereEveryoneGoes);

        return correctJoinTuple->getMerger(partitionId);
    }

    // JiaNote: this gets a sink shuffler
    SinkShufflerPtr getSinkShuffler(TupleSpec& consumeMe,
                                    TupleSpec& attsToOpOn,
                                    TupleSpec& projection,
                                    ComputePlan& plan) override {
        std :: cout << "to get sink shuffler" << std :: endl;
        // loop through each of the attributes that we are supposed to accept, and for each of them,
        // find the type
        std::vector<std::string> typeList;
        AtomicComputationPtr producer =
            plan.getPlan()->getComputations().getProducingAtomicComputation(consumeMe.getSetName());
        for (auto& a : projection.getAtts()) {

            // find the identity of the producing computation
            std::pair<std::string, std::string> res =
                producer->findSource(a, plan.getPlan()->getComputations());

            // and find its type... in the first case, there is not a particular lambda that we need
            // to ask for
            if (res.second == "") {
                typeList.push_back(
                    "pdb::Handle<" +
                    plan.getPlan()->getNode(res.first).getComputation().getOutputType() + ">");
            } else {
                std::string myType =
                    plan.getPlan()->getNode(res.first).getLambda(res.second)->getOutputType();

                if (myType.find_first_of("pdb::Handle<") == 0) {
                    typeList.push_back(myType);
                } else {
                    typeList.push_back("pdb::Handle<" + myType + ">");
                }
            }
        }

        for (auto &aa : typeList) {
                std :: cout << "Got type " << aa << "\n";
        }

        // now we get the correct join tuple, that will allow us to pack tuples from the join in a
        // hash table
        std::vector<int> whereEveryoneGoes;
        JoinTuplePtr correctJoinTuple =
            findCorrectJoinTuple<In1, In2, Rest...>(typeList, whereEveryoneGoes);

        return correctJoinTuple->getShuffler();
    }


    // this gets a compute sink
    ComputeSinkPtr getComputeSink(TupleSpec& consumeMe,
                                  TupleSpec& attsToOpOn,
                                  TupleSpec& projection,
                                  ComputePlan& plan) override {
        std :: cout << "to get compute sink for Join" << std :: endl;
        // loop through each of the attributes that we are supposed to accept, and for each of them,
        // find the type
        std::vector<std::string> typeList;
        AtomicComputationPtr producer =
            plan.getPlan()->getComputations().getProducingAtomicComputation(consumeMe.getSetName());
        std :: cout << "consumeMe was: " << consumeMe << "\n";
        std :: cout << "attsToOpOn was: " << attsToOpOn << "\n";
        std :: cout << "getComputeSink: projection was: " << projection << "\n";
        
        for (auto& a : projection.getAtts()) {

            // find the identity of the producing computation
            // std :: cout << "finding the source of " << projection.getSetName () << "." << a <<
            // "\n";
            std::pair<std::string, std::string> res =
                producer->findSource(a, plan.getPlan()->getComputations());
            // std :: cout << "got " << res.first << " " << res.second << "\n";

            // and find its type... in the first case, there is not a particular lambda that we need
            // to ask for
            if (res.second == "") {
                typeList.push_back(
                    "pdb::Handle<" +
                    plan.getPlan()->getNode(res.first).getComputation().getOutputType() + ">");
            } else {
                std::string myType =
                    plan.getPlan()->getNode(res.first).getLambda(res.second)->getOutputType();
                // std :: cout << "my type is " << myType << std :: endl;
                if (myType.find_first_of("pdb::Handle<") == 0) {
                    typeList.push_back(myType);
                } else {
                    typeList.push_back("pdb::Handle<" + myType + ">");
                }
            }
        }

        for (auto &aa : typeList) {
            std :: cout << "Got type " << aa << "\n";
        }

        // now we get the correct join tuple, that will allow us to pack tuples from the join in a
        // hash table
        std::vector<int> whereEveryoneGoes;
        JoinTuplePtr correctJoinTuple =
            findCorrectJoinTuple<In1, In2, Rest...>(typeList, whereEveryoneGoes);

        if ((this->joinType == BroadcastJoin) || (this->joinType == LocalJoin)) {
            return correctJoinTuple->getSink(consumeMe, attsToOpOn, projection, whereEveryoneGoes);
        } else if (this->joinType == HashPartitionedJoin) {
            return correctJoinTuple->getPartitionedSink(numPartitions / numNodes,
                                                        numNodes,
                                                        consumeMe,
                                                        attsToOpOn,
                                                        projection,
                                                        whereEveryoneGoes);
        } else {
            return nullptr;
        }
    }

    // JiaNote: to get compute source for HashPartitionedJoin
    ComputeSourcePtr getComputeSource(TupleSpec& outputScheme, ComputePlan& plan) override {
        std::cout << "to get compute source" << std::endl;
        if ((this->joinType != HashPartitionedJoin) && (this->joinType != LocalJoin)) {
            return nullptr;
        }
        // loop through each of the attributes that we are supposed to accept, and for each of them,
        // find the type
        std :: cout << "outputScheme is " << outputScheme << std :: endl;
        std::vector<std::string> typeList;
        AtomicComputationPtr producer =
            plan.getPlan()->getComputations().getProducingAtomicComputation(
                outputScheme.getSetName());
        for (auto& a : outputScheme.getAtts()) {
            if ((a.find("hash") != std::string::npos)||(a.find("OneFor") != std::string::npos)) {
                continue;
            }
            // find the identity of the producing computation
            std::pair<std::string, std::string> res =
                producer->findSource(a, plan.getPlan()->getComputations());
            std :: cout << "got " << res.first << " " << res.second << "\n";

            // and find its type... in the first case, there is not a particular lambda that we need
            // to ask for
            if (res.second == "") {

                typeList.push_back(
                    "pdb::Handle<" +
                    plan.getPlan()->getNode(res.first).getComputation().getOutputType() + ">");
            } else {
                std::string myType =
                    plan.getPlan()->getNode(res.first).getLambda(res.second)->getOutputType();
                if (myType.find_first_of("pdb::Handle<") == 0) {
                    typeList.push_back(myType);
                } else {
                    typeList.push_back("pdb::Handle<" + myType + ">");
                }
            }
        }
        for (auto &aa : typeList) {
                std :: cout << "Got type " << aa << "\n";
        }

        // now we get the correct join tuple, that will allow us to pack tuples from the join in a
        // hash table
        std::vector<int> whereEveryoneGoes;
        JoinTuplePtr correctJoinTuple =
            findCorrectJoinTuple<In1, In2, Rest...>(typeList, whereEveryoneGoes);

        if (this->joinType == HashPartitionedJoin){

            return correctJoinTuple->getPartitionedSource( this->myPartitionId, 

            [&]() -> PDBPagePtr {
                if (this->iterator == nullptr) {
                    std::cout << "Error in JoinComp: partitioned join source has a null iterator"
                              << std::endl;
                    return nullptr;
                }
                while (this->iterator->hasNext() == true) {

                    PDBPagePtr page = this->iterator->next();
                    if (page != nullptr) {
                        return page;
                    }
                }

                return nullptr;

            },

            [&](PDBPagePtr freeMe) -> void {
                if (this->proxy != nullptr) {
                    char* curBytes = freeMe->getRawBytes();
                    NodeID nodeId = (NodeID)(*((NodeID*)(curBytes)));
                    curBytes = curBytes + sizeof(NodeID);
                    DatabaseID dbId = (DatabaseID)(*((DatabaseID*)(curBytes)));
                    curBytes = curBytes + sizeof(DatabaseID);
                    UserTypeID typeId = (UserTypeID)(*((UserTypeID*)(curBytes)));
                    curBytes = curBytes + sizeof(UserTypeID);
                    SetID setId = (SetID)(*((SetID*)(curBytes)));
                    freeMe->decRefCount();
                    if (freeMe->getRefCount() == 0) {
#ifdef PROFILING_CACHE
                        std::cout << "To unpin Join source page with DatabaseID=" << dbId
                                  << ", UserTypeID=" << typeId << ", SetID=" << setId
                                  << ", PageID=" << freeMe->getPageID() << std::endl;
#endif
                        try {
                            this->proxy->unpinUserPage(nodeId, dbId, typeId, setId, freeMe, false);
                        } catch (NotEnoughSpace& n) {
                            makeObjectAllocatorBlock(4096, true);
                            this->proxy->unpinUserPage(nodeId, dbId, typeId, setId, freeMe, false);
                            throw n;
                        }
                    }
#ifdef PROFILING_CACHE
                    else {
                        std::cout << "Can't unpin Join source page with DatabaseID=" << dbId
                                  << ", UserTypeID=" << typeId << ", SetID=" << setId
                                  << ", PageID=" << freeMe->getPageID()
                                  << ", reference count=" << freeMe->getRefCount() << std::endl;
                    }
#endif
                }
            },

            this->batchSize,

            whereEveryoneGoes

            );
        } else {
            return nullptr;
        }

    }


    // this is a join computation
    std::string getComputationType() override {
        return std::string("JoinComp");
    }

    // JiaNote: Returning a TCAP string for this Join computation
    virtual std::string toTCAPString(std::vector<InputTupleSetSpecifier>& inputTupleSets,
                                     int computationLabel,
                                     std::string& outputTupleSetName,
                                     std::vector<std::string>& outputColumnNames,
                                     std::string& addedOutputColumnName) override {

        this->setComputationName(getComputationType()+"_"+std::to_string(computationLabel));
        if (inputTupleSets.size() == getNumInputs()) {
            std::string tcapString = "";
            if (multiInputsBase == nullptr) {
                multiInputsBase = new MultiInputsBase();
            }
            multiInputsBase->setNumInputs(this->getNumInputs());
            std::vector<std::string> inputNames;
            // update tupleset name for input sets
            for (unsigned int i = 0; i < inputTupleSets.size(); i++) {
                this->multiInputsBase->setTupleSetNameForIthInput(
                    i, inputTupleSets[i].getTupleSetName());
                this->multiInputsBase->setInputColumnsForIthInput(
                    i, inputTupleSets[i].getColumnNamesToKeep());
                this->multiInputsBase->setInputColumnsToApplyForIthInput(
                    i, inputTupleSets[i].getColumnNamesToApply());
                std::cout << "JoinComp: setInputColumnsToApplyForIthInput: " << i << ":" << inputTupleSets[i].getColumnNamesToApply()[0] << std::endl;
                inputNames.push_back(inputTupleSets[i].getColumnNamesToApply()[0]);
            }
            analyzeInputSets(inputNames);
            Lambda<bool> selectionLambda = callGetSelection(*this);
            std::string inputTupleSetName = "";
            std::vector<std::string> inputColumnNames;
            std::vector<std::string> inputColumnsToApply;
            std::vector<std::string> childrenLambdaNames;
            int lambdaLabel = 0;
            std::string myLambdaName;
            MultiInputsBase* multiInputsComp = this->getMultiInputsBase();
            tcapString += selectionLambda.toTCAPString(inputTupleSetName,
                                                       inputColumnNames,
                                                       inputColumnsToApply,
                                                       childrenLambdaNames,
                                                       lambdaLabel,
                                                       "JoinComp",
                                                       computationLabel,
                                                       outputTupleSetName,
                                                       outputColumnNames,
                                                       addedOutputColumnName,
                                                       myLambdaName,
                                                       false,
                                                       multiInputsComp,
                                                       true);


            std::vector<std::string> inputsInProjection = multiInputsComp->getInputsInProjection();
            tcapString += "\n/* run Join projection on ( " + inputsInProjection[0];
            for (unsigned int i = 1; i < inputsInProjection.size(); i++) {
                tcapString += " " + inputsInProjection[i];
            }
            tcapString += " )*/\n";
            Lambda<Handle<Out>> projectionLambda = callGetProjection(*this);
            inputTupleSetName = outputTupleSetName;
            inputColumnNames.clear();
            inputColumnsToApply.clear();
            childrenLambdaNames.clear();
            for (unsigned int index = 0; index < multiInputsComp->getNumInputs(); index++) {
                multiInputsComp->setInputColumnsForIthInput(index, inputColumnNames);
            }

            tcapString += projectionLambda.toTCAPString(inputTupleSetName,
                                                        inputColumnNames,
                                                        inputColumnsToApply,
                                                        childrenLambdaNames,
                                                        lambdaLabel,
                                                        "JoinComp",
                                                        computationLabel,
                                                        outputTupleSetName,
                                                        outputColumnNames,
                                                        addedOutputColumnName,
                                                        myLambdaName,
                                                        true,
                                                        multiInputsComp,
                                                        false);
            this->setOutputTupleSetName(outputTupleSetName);
            this->setOutputColumnToApply(addedOutputColumnName);
            setMultiInputsBaseToNull();
            return tcapString;

        } else {
            std::cout << "ERROR: inputTupleSet size is " << inputTupleSets.size()
                      << " and not equivalent with Join's inputs " << getNumInputs() << std::endl;
            return "";
        }
    }


    // gets an execute that can run a scan join... needToSwapAtts is true if the atts that are
    // currently stored in the hash table need to
    // come SECOND in the output tuple sets... hashedInputSchema tells us the schema for the
    // attributes that are currently stored in the
    // hash table... pipelinedInputSchema tells us the schema for the attributes that will be coming
    // through the pipeline...
    // pipelinedAttsToOperateOn is the identity of the hash attribute...
    // pipelinedAttsToIncludeInOutput tells us the set of attributes
    // that are coming through the pipeline that we actually have to write to the output stream
    ComputeExecutorPtr getExecutor(bool needToSwapAtts,
                                   TupleSpec& hashedInputSchema,
                                   TupleSpec& pipelinedInputSchema,
                                   TupleSpec& pipelinedAttsToOperateOn,
                                   TupleSpec& pipelinedAttsToIncludeInOutput,
                                   ComputeInfoPtr arg) override {
        std::cout << "to get executor" << std::endl;
        // get the argument to the join
        JoinArg& joinArg = *((JoinArg*)arg.get());


        // loop through each of the attributes that we are supposed to accept, and for each of them,
        // find the type
        std::vector<std::string> typeList;
        AtomicComputationPtr producer =
            joinArg.plan.getPlan()->getComputations().getProducingAtomicComputation(
                hashedInputSchema.getSetName());
        for (auto& a : (hashedInputSchema.getAtts())) {

            std::pair<std::string, std::string> res =
                producer->findSource(a, joinArg.plan.getPlan()->getComputations());

            // and find its type... in the first case, there is not a particular lambda that we need
            // to ask for
            if (res.second == "") {
                typeList.push_back(
                    "pdb::Handle<" +
                    joinArg.plan.getPlan()->getNode(res.first).getComputation().getOutputType() +
                    ">");
            } else {
                std::string myType = joinArg.plan.getPlan()
                                         ->getNode(res.first)
                                         .getLambda(res.second)
                                         ->getOutputType();
                if (myType.find_first_of("pdb::Handle<") == 0) {
                    typeList.push_back(myType);
                } else {
                    typeList.push_back("pdb::Handle<" + myType + ">");
                }
            }
        }
        for (auto &aa : typeList) {
                std :: cout << "Got type " << aa << "\n";
        }

        // now we get the correct join tuple, that will allow us to pack tuples from the join in a
        // hash table
        std::vector<int> whereEveryoneGoes;
        JoinTuplePtr correctJoinTuple =
            findCorrectJoinTuple<In1, In2, Rest...>(typeList, whereEveryoneGoes);

        // and return the correct probing code

        if (joinArg.partitionedHashSet == nullptr) {

            return correctJoinTuple->getProber(joinArg.pageWhereHashTableIs,
                                           whereEveryoneGoes,
                                           pipelinedInputSchema,
                                           pipelinedAttsToOperateOn,
                                           pipelinedAttsToIncludeInOutput,
                                           needToSwapAtts);
        } else {

            return correctJoinTuple->getPartitionedProber(joinArg.partitionedHashSet,
                                                          numPartitions / numNodes,
                                                          numNodes,
                                                          whereEveryoneGoes,
                                                          pipelinedInputSchema,
                                                          pipelinedAttsToOperateOn,
                                                          pipelinedAttsToIncludeInOutput,
                                                          needToSwapAtts);

        }
    }

    ComputeExecutorPtr getExecutor(bool needToSwapAtts,
                                   TupleSpec& hashedInputSchema,
                                   TupleSpec& pipelinedInputSchema,
                                   TupleSpec& pipelinedAttsToOperateOn,
                                   TupleSpec& pipelinedAttsToIncludeInOutput) override {
        std::cout << "Currently, no pipelined version of the join doesn't take an arg.\n";
        exit(1);
    }
};
}

#endif
