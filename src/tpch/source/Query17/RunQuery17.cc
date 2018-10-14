#ifndef RUN_QUERY17_CC
#define RUN_QUERY17_CC

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
#include "Query17.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>


using namespace std;
using namespace tpch;

/*
    val mul02 = udf { (x: Double) => x * 0.2 }

    val flineitem = lineitem.select($"l_partkey", $"l_quantity", $"l_extendedprice")

    val fpart = part.filter($"p_brand" === "Brand#23" && $"p_container" === "MED BOX")
      .select($"p_partkey")
      .join(lineitem, $"p_partkey" === lineitem("l_partkey"), "left_outer")
    // select

    fpart.groupBy("p_partkey")
      .agg(mul02(avg($"l_quantity")).as("avg_quantity"))
      .select($"p_partkey".as("key"), $"avg_quantity")
      .join(fpart, $"key" === fpart("p_partkey"))
      .filter($"l_quantity" < $"avg_quantity")
      .agg(sum($"l_extendedprice") / 7.0)

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
        pdbClient.registerType ("libraries/libQ17JoinedPartLineItem.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PartLineItemAvgJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PartSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ17LineItemAvgQuantity.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PartLineItemJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PriceSum.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PartLineItemIdentitySelection.so", errMsg);
    }    


    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q17_output_set", errMsg);
    if (!pdbClient.createSet<DoubleSumResult>(
            "tpch", "q17_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    // make the query graph
    Handle<Computation> myPartScanner = makeObject<ScanUserSet<Part>>("tpch", "part");
    Handle<Computation> myLineItemScanner = makeObject<ScanUserSet<LineItem>>("tpch", "lineitem");

    Handle<Computation> myQ17PartSelection = makeObject<Q17PartSelection>("Brand#23", "MED BOX");
    Handle<Computation> myQ17PartLineItemJoin = makeObject<Q17PartLineItemJoin>();
    Handle<Computation> myQ17PartLineItemIdentitySelection = 
         makeObject<Q17PartLineItemIdentitySelection> ();
    Handle<Computation> myQ17LineItemAvgQuantity = makeObject<Q17LineItemAvgQuantity>();
    Handle<Computation> myQ17PartLineItemAvgJoin = makeObject<Q17PartLineItemAvgJoin>();
    Handle<Computation> myQ17PriceSum = makeObject<Q17PriceSum>();
    Handle<Computation> myWriteSet = makeObject<WriteUserSet<DoubleSumResult>>("tpch", "q17_output_set");

    myQ17PartSelection->setInput(myPartScanner);
    myQ17PartLineItemJoin->setInput(0, myQ17PartSelection);
    myQ17PartLineItemJoin->setInput(1, myLineItemScanner);
    myQ17PartLineItemIdentitySelection->setInput(myQ17PartLineItemJoin);
    myQ17LineItemAvgQuantity->setInput(myQ17PartLineItemIdentitySelection);
    myQ17PartLineItemAvgJoin->setInput(0, myQ17PartLineItemIdentitySelection);
    myQ17PartLineItemAvgJoin->setInput(1, myQ17LineItemAvgQuantity);
    myQ17PriceSum->setInput(myQ17PartLineItemAvgJoin);
    myWriteSet->setInput(myQ17PriceSum);

    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery17", myWriteSet)) {
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


    SetIterator<DoubleSumResult> result =
            pdbClient.getSetIterator<DoubleSumResult>("tpch", "q17_output_set");

    std::cout << "Query results: ";
    int count = 0;
    for (auto a : result) {
        DoubleSumResult r = *a;
        std::cout << "sum is " << r.total << std::endl;
        count++;
    }
    std::cout << "Output count:" << count << "\n";
    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q17_output_set", errMsg)) {
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
