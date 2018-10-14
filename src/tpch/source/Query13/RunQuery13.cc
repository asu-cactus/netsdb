#ifndef RUN_QUERY13_CC
#define RUN_QUERY13_CC

#include "CatalogClient.h"

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <chrono>
#include <sstream>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "DataTypes.h"
#include "InterfaceFunctions.h"
#include "Handle.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"
#include "QueryOutput.h"
#include "ScanUserSet.h"
#include "WriteUserSet.h"

#include "TPCHSchema.h"
#include "Query13.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>


using namespace std;
using namespace tpch;

/*
    val special = udf { (x: String) => x.matches(".*special.*requests.*") }

    customer.join(order, $"c_custkey" === order("o_custkey")
      && !special(order("o_comment")), "left_outer")
      .groupBy($"o_custkey")
      .agg(count($"o_orderkey").as("c_count"))
      .groupBy($"c_count")
      .agg(count($"o_custkey").as("custdist"))
      .sort($"custdist".desc, $"c_count".desc)
*/


int main(int argc, char* argv[]) {

    bool whetherToRegisterLibraries = false;
    if (argc > 1) {
        if (strcmp(argv[1], "Y") == 0) {
            whetherToRegisterLibraries = true;
        }
    }


    // Connection info
    string masterHostname = "localhost";
    int masterPort = 8108;

    // register the shared employee class
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");

    PDBClient pdbClient(
            masterPort, masterHostname,
            clientLogger,
            false,
            true);

    CatalogClient catalogClient(
            masterPort,
            masterHostname,
            clientLogger);

    if (whetherToRegisterLibraries == true) {
        std::string errMsg;
        pdbClient.registerType ("libraries/libQ13CountResult.so", errMsg);
        pdbClient.registerType ("libraries/libQ13CustomerOrderJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ13OrderSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ13CustomerDistribution.so", errMsg);
        pdbClient.registerType ("libraries/libQ13CustomerOrders.so", errMsg);
        pdbClient.registerType ("libraries/libQ13OrdersPerCustomer.so", errMsg);
    }    


    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q13_output_set", errMsg);
    if (!pdbClient.createSet<Q13CountResult>(
            "tpch", "q13_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    // make the query graph
    Handle<Computation> myCustomerScanner = makeObject<ScanUserSet<Customer>>("tpch", "customer");
    Handle<Computation> myOrderScanner = makeObject<ScanUserSet<Order>>("tpch", "order");

    Handle<Computation> myQ13OrderSelection = makeObject<Q13OrderSelection>("special", "request");
    Handle<Computation> myQ13CustomerOrderJoin = makeObject<Q13CustomerOrderJoin>();
    Handle<Computation> myQ13OrdersPerCustomer = makeObject<Q13OrdersPerCustomer>();
    Handle<Computation> myQ13CustomerDistribution = makeObject<Q13CustomerDistribution>();
    Handle<Computation> myWriteSet = makeObject<WriteUserSet<Q13CountResult>>("tpch", "q13_output_set");

    myQ13OrderSelection->setInput(myOrderScanner);
    myQ13CustomerOrderJoin->setInput(0, myCustomerScanner);
    myQ13CustomerOrderJoin->setInput(1, myQ13OrderSelection);
    myQ13OrdersPerCustomer->setInput(myQ13CustomerOrderJoin);
    myQ13CustomerDistribution->setInput(myQ13OrdersPerCustomer);
    myWriteSet->setInput(myQ13CustomerDistribution);

    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery13", myWriteSet)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }

    std::cout << std::endl;
    auto end = std::chrono::high_resolution_clock::now();

    float timeDifference =
        (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count())) /
        (float)1000000000;



    // Printing results to double check
    std::cout << "to print result..." << std::endl;


    SetIterator<Q13CountResult> result =
            pdbClient.getSetIterator<Q13CountResult>("tpch", "q13_output_set");

    std::cout << "Query results: ";
    int count = 0;
    for (auto a : result) {
        Q13CountResult r = *a;
        std::cout << "there are " << r.value << " customers have " << r.key << " orders" << std::endl;
        count++;
    }
    std::cout << "Output count:" << count << "\n";
    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q13_output_set", errMsg)) {
        cout << "Not able to remove the set: " + errMsg;
        exit(-1);
    } else {
        cout << "Set removed. \n";
    }

    // Clean up the SO files.
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {

        std::cout << "Can't cleanup so files" << std::endl;
    }

}
#endif
