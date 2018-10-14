#ifndef RUN_QUERY02_CC
#define RUN_QUERY02_CC

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
#include "Query02.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>


using namespace std;
using namespace tpch;

/*
    val europe = region.filter($"r_name" === "EUROPE")
      .join(nation, $"r_regionkey" === nation("n_regionkey"))
      .join(supplier, $"n_nationkey" === supplier("s_nationkey"))
      .join(partsupp, supplier("s_suppkey") === partsupp("ps_suppkey"))
    //.select($"r_regionkey", $"n_regionkey", $"s_suppkey", $"n_nationkey", $"s_nationkey", $"p_partkey", $"p_mfgr", $"ps_partkey", $"ps_supplycost", $"s_acctbal", $"s_name", $"n_name", $"s_address", $"s_phone", $"s_comment")

    val brass = part.filter(part("p_size") === 15 && part("p_type").endsWith("BRASS"))
      .join(europe, europe("ps_partkey") === $"p_partkey")
    //.cache

    val minCost = brass.groupBy(brass("ps_partkey"))
      .agg(min("ps_supplycost").as("min"))

    brass.join(minCost, brass("ps_partkey") === minCost("ps_partkey"))
      .filter(brass("ps_supplycost") === minCost("min"))
      .select("s_acctbal", "s_name", "n_name", "p_partkey", "p_mfgr", "s_address", "s_phone", "s_comment")
      .sort($"s_acctbal".desc, $"n_name", $"s_name", $"p_partkey")
      .limit(100)
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
        pdbClient.registerType ("libraries/libMinDouble.so", errMsg);
        pdbClient.registerType ("libraries/libQ02MinAgg.so", errMsg);
        pdbClient.registerType ("libraries/libQ02MinCostJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ02MinCostJoinOutput.so", errMsg);
        pdbClient.registerType ("libraries/libQ02MinCostPerPart.so", errMsg);
        pdbClient.registerType ("libraries/libQ02MinCostSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ02MinCostSelectionOutput.so", errMsg);
        pdbClient.registerType ("libraries/libQ02NationJoin.so", errMsg); 
        pdbClient.registerType ("libraries/libQ02PartJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ02PartJoinOutput.so", errMsg);
        pdbClient.registerType ("libraries/libQ02PartSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ02PartSuppJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ02PartSuppJoinOutput.so", errMsg);
        pdbClient.registerType ("libraries/libQ02RegionSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ02SupplierJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ02SupplierJoinOutput.so", errMsg);      
        pdbClient.registerType ("libraries/libQ02PartJoinOutputIdentitySelection.so", errMsg);
    }    


    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q02_output_set", errMsg);
    if (!pdbClient.createSet<Q02MinCostSelectionOutput>(
            "tpch", "q02_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    // make the query graph
    //Handle<Computation> myCustomerScanner = makeObject<ScanUserSet<Customer>>("tpch", "customer");
    //Handle<Computation> myLineItemScanner = makeObject<ScanUserSet<LineItem>>("tpch", "lineitem");
    Handle<Computation> myNationScanner = makeObject<ScanUserSet<Nation>>("tpch", "nation");
    //Handle<Computation> myOrderScanner = makeObject<ScanUserSet<Order>>("tpch", "order");
    Handle<Computation> myPartScanner = makeObject<ScanUserSet<Part>>("tpch", "part");
    Handle<Computation> myPartSuppScanner = makeObject<ScanUserSet<PartSupp>>("tpch", "partsupp");
    Handle<Computation> myRegionScanner = makeObject<ScanUserSet<Region>>("tpch", "region");
    Handle<Computation> mySupplierScanner = makeObject<ScanUserSet<Supplier>>("tpch", "supplier");

    Handle<Computation> myQ02RegionSelection = makeObject<Q02RegionSelection>("EUROPE");
    Handle<Computation> myQ02NationJoin = makeObject<Q02NationJoin>();
    Handle<Computation> myQ02SupplierJoin = makeObject<Q02SupplierJoin>();
    Handle<Computation> myQ02PartSuppJoin = makeObject<Q02PartSuppJoin>();
    Handle<Computation> myQ02PartSelection = makeObject<Q02PartSelection>(15, "BRASS");
    Handle<Computation> myQ02PartJoin = makeObject<Q02PartJoin>();
    Handle<Computation> myQ02PartJoinOutputSelection = makeObject<Q02PartJoinOutputIdentitySelection>();
    Handle<Computation> myQ02MinAgg = makeObject<Q02MinAgg>();
    Handle<Computation> myQ02MinCostJoin = makeObject<Q02MinCostJoin>();
    Handle<Computation> myQ02MinCostSelection = makeObject<Q02MinCostSelection>();

    Handle<Computation> myQ02MinCostSelectionOutputWriter = makeObject<WriteUserSet<Q02MinCostSelectionOutput>> ("tpch", "q02_output_set");

    myQ02RegionSelection->setInput(myRegionScanner);
    myQ02NationJoin->setInput(0, myQ02RegionSelection);
    myQ02NationJoin->setInput(1, myNationScanner);
    myQ02SupplierJoin->setInput(0, myQ02NationJoin);
    myQ02SupplierJoin->setInput(1, mySupplierScanner);
    myQ02PartSuppJoin->setInput(0, myQ02SupplierJoin);
    myQ02PartSuppJoin->setInput(1, myPartSuppScanner);
    
    myQ02PartSelection->setInput(myPartScanner);
    myQ02PartJoin->setInput(0, myQ02PartSelection);
    myQ02PartJoin->setInput(1, myQ02PartSuppJoin);
    myQ02PartJoinOutputSelection->setInput(myQ02PartJoin);
    
    myQ02MinAgg->setInput(myQ02PartJoinOutputSelection);
    myQ02MinCostJoin->setInput(0, myQ02MinAgg);
    myQ02MinCostJoin->setInput(1, myQ02PartJoinOutputSelection);
    myQ02MinCostSelection->setInput(myQ02MinCostJoin);
    myQ02MinCostSelectionOutputWriter->setInput(myQ02MinCostSelection);


    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery02", myQ02MinCostSelectionOutputWriter)) {
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


    SetIterator<Q02MinCostSelectionOutput> result =
            pdbClient.getSetIterator<Q02MinCostSelectionOutput>("tpch", "q02_output_set");

    std::cout << "Query results: ";
    int count = 0;
    for (auto a : result) {
        Q02MinCostSelectionOutput r = *a;
        std::cout << "p_partkey=" << r.p_partkey << ", p_mfgr=" << r.p_mfgr << ", n_name=" << r.n_name 
            << ", s_acctbal=" << r.s_acctbal << ", s_name=" << r.s_name << ", s_address=" << r.s_address
            << ", s_phone=" << r.s_phone << ", s_comment=" << r.s_comment << std::endl;
        count++;
    }
    std::cout << "Output count:" << count << "\n";
    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q02_output_set", errMsg)) {
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
