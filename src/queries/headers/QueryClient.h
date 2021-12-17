
#ifndef QUERY_CLIENT
#define QUERY_CLIENT

#include "Set.h"
#include "SetIterator.h"
#include "Handle.h"
#include "PDBLogger.h"
#include "PDBVector.h"
#include "CatalogClient.h"
#include "DeleteSet.h"
#include "ExecuteQuery.h"
#include "TupleSetExecuteQuery.h"
#include "ExecuteComputation.h"
#include "QueryGraphAnalyzer.h"
#include "Computation.h"
namespace pdb {

class QueryClient {

public:
    QueryClient() {}

    // connect to the database
    QueryClient(int portIn,
                std::string addressIn,
                PDBLoggerPtr myLoggerIn,
                bool useScheduler = false,
                std::string clientName = "")
        : myHelper(portIn, addressIn, myLoggerIn) {
        port = portIn;
        address = addressIn;
        myLogger = myLoggerIn;
        runUs = makeObject<Vector<Handle<QueryBase>>>();
        queryGraph = makeObject<Vector<Handle<Computation>>>();
        this->useScheduler = useScheduler;
        this->clientName = clientName;
        this->queryId = 0;
    }

    ~QueryClient() {
        runUs = nullptr;
        queryGraph = nullptr;
    }

    // access a set in the database
    template <class Type>
    Handle<Set<Type>> getSet(std::string databaseName, std::string setName) {

// verify that the database and set work
#ifdef DEBUG_SET_TYPE
        std::string errMsg;

        std::string typeName = myHelper.getObjectType(databaseName, setName, errMsg);

        if (typeName == "") {
            std::cout << "I was not able to obtain the type for database set " << setName << "\n";
            myLogger->error("query client: not able to verify type: " + errMsg);
            Handle<Set<Type>> returnVal = makeObject<Set<Type>>(false);
            return returnVal;
        }

        if (typeName != getTypeName<Type>()) {
            std::cout << "Wrong type for database set " << setName << "\n";
            Handle<Set<Type>> returnVal = makeObject<Set<Type>>(false);
            return returnVal;
        }
#endif
        Handle<Set<Type>> returnVal = makeObject<Set<Type>>(databaseName, setName);
        return returnVal;
    }

    // get an iterator for a set in the database
    template <class Type>
    SetIterator<Type> getSetIterator(std::string databaseName, std::string setName, bool isShared = false) {

// verify that the database and set work
#ifdef DEBUG_SET_TYPE
        std::string errMsg;
        std::string typeName = myHelper.getObjectType(databaseName, setName, errMsg);

        if (typeName == "") {
            myLogger->error("query client: not able to verify type: " + errMsg);
            SetIterator<Type> returnVal;
            return returnVal;
        }
#endif
        // commented by Jia, below type check can not work with complex types such as
        // Vector<Handle<Foo>>
        /*
if (typeName != getTypeName <Type> ()) {
    std :: cout << "Wrong type for database set " << setName << "\n";
    SetIterator <Type> returnVal;
    return returnVal;
}
        */
        SetIterator<Type> returnVal(myLogger, port, address, databaseName, setName, isShared);
        return returnVal;
    }

    bool deleteSet(std::string databaseName, std::string setName) {
        // this is for query testing stuff
        return simpleRequest<DeleteSet, SimpleRequestResult, bool, String, String>(
            myLogger,
            port,
            address,
            false,
            124 * 1024,
            [&](Handle<SimpleRequestResult> result) {
                std::string errMsg;
                if (result != nullptr) {

                    // make sure we got the correct number of results
                    if (!result->getRes().first) {
                        errMsg = "Could not remove set: " + result->getRes().second;
                        myLogger->error("QueryErr: " + errMsg);
                        return false;
                    }

                    return true;
                }
                errMsg = "Error getting type name: got nothing back from catalog";
                return false;
            },
            databaseName,
            setName);
    }

    // JiaNote: to execute computations
    template <class... Types>
    bool executeComputations(std::string& errMsg,
                             Handle<Computation> firstParam,
                             Handle<Types>... args) {
        queryGraph->push_back(firstParam);
        return executeComputations(errMsg, args...);
    }


    template <class... Types>
    bool executeComputations(std::string& errMsg,
                             std::string jobName,
                             Handle<Computation> firstParam,
                             Handle<Types>... args) {
        queryGraph->push_back(firstParam);
        return executeComputations(errMsg, jobName, args...);
    }


    template <class... Types>
    bool executeComputations(std::string& errMsg,
                             std::string jobName,
                             bool preCompile,
                             Handle<Computation> firstParam,
                             Handle<Types>... args) {
        queryGraph->push_back(firstParam);
        return executeComputations(errMsg, jobName, preCompile, args...);
    }


    bool executeComputations(std::string& errMsg, std::string jobName = "", bool preCompile = false) {

        // this is the request
        const UseTemporaryAllocationBlock myBlock{256 * 1024 * 1024};
        QueryGraphAnalyzer queryAnalyzer(this->queryGraph);
        std::string tcapString = queryAnalyzer.parseTCAPString();
        std::vector<Handle<Computation>> computations;
        queryAnalyzer.parseComputations(computations);
        Handle<Vector<Handle<Computation>>> computationsToSend =
            makeObject<Vector<Handle<Computation>>>();
        for (int i = 0; i < computations.size(); i++) {
            computationsToSend->push_back(computations[i]);
        }
        
        if (jobName == "") {
            jobName = clientName + "-" + std::to_string(queryId);
            queryId ++;
        }
        std::cout << "jobName is " << jobName << std::endl;
        Handle<ExecuteComputation> executeComputation = makeObject<ExecuteComputation>(
            jobName, tcapString, preCompile);

        // this call asks the database to execute the query, and then it inserts the result set name
        // within each of the results, as well as the database connection information

        // this is for query scheduling stuff
        if (useScheduler == true) {
            return simpleDoubleRequest<ExecuteComputation,
                                       Vector<Handle<Computation>>,
                                       SimpleRequestResult,
                                       bool>(
                myLogger,
                port,
                address,
                false,
                124 * 1024,
                [&](Handle<SimpleRequestResult> result) {
                    if (result != nullptr) {
                        if (!result->getRes().first) {
                            errMsg = "Error in query: " + result->getRes().second;
                            myLogger->error("Error querying data: " + result->getRes().second);
                            return false;
                        }
                        this->queryGraph = makeObject<Vector<Handle<Computation>>>();
                        return true;
                    }
                    errMsg = "Error getting type name: got nothing back from server";
                    this->queryGraph = makeObject<Vector<Handle<Computation>>>();
                    return false;


                },
                executeComputation,
                computationsToSend);


        } else {
            errMsg =
                "This query must be sent to QuerySchedulerServer, but it seems "
                "QuerySchedulerServer is not supported";
            this->queryGraph = makeObject<Vector<Handle<Computation>>>();
            return false;
        }
        this->queryGraph = makeObject<Vector<Handle<Computation>>>();
    }


    void setUseScheduler(bool useScheduler) {
        this->useScheduler = useScheduler;
    }

private:
    // how we connect to the catalog
    CatalogClient myHelper;

    // deprecated
    // this is the query graph we'll execute
    Handle<Vector<Handle<QueryBase>>> runUs;

    // JiaNote: the Computation-based query graph to execute
    Handle<Vector<Handle<Computation>>> queryGraph;


    // connection info
    int port;
    std::string address;

    // for logging
    PDBLoggerPtr myLogger;

    // JiaNote: whether to run in distributed mode
    bool useScheduler;

    // JiaNote: the client name
    std::string clientName;

    // JiaNote: the query id
    int queryId;


};
}

#endif
