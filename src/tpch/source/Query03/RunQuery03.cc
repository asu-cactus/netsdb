#ifndef RUN_QUERY03_CC
#define RUN_QUERY03_CC

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
#include "Query03.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>


using namespace std;
using namespace tpch;

/*
select
l_orderkey,
sum(l_extendedprice*(1-l_discount)) as revenue, o_orderdate,
o_shippriority
from
customer,
orders,
lineitem where
c_mktsegment = '[SEGMENT]' and c_custkey = o_custkey
and l_orderkey = o_orderkey and o_orderdate < date '[DATE]' and l_shipdate > date '[DATE]'
group by l_orderkey,
o_orderdate,
o_shippriority order by
revenue desc, o_orderdate;
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
        pdbClient.registerType ("libraries/libQ03Agg.so", errMsg);
        pdbClient.registerType ("libraries/libQ03AggOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ03Join.so", errMsg);
        pdbClient.registerType ("libraries/libQ03JoinOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ03CustomerSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ03OrderSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ03LineItemSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ03KeyClass.so", errMsg);
    }    


    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q03_output_set", errMsg);
    if (!pdbClient.createSet<Q03AggOut>(
            "tpch", "q03_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    // make the query graph
    Handle<Computation> myCustomerScanner = makeObject<ScanUserSet<Customer>>("tpch", "customer");

    Handle<Computation> myOrderScanner = makeObject<ScanUserSet<Order>>("tpch", "order");
    Handle<Computation> myLineItemScanner = makeObject<ScanUserSet<LineItem>>("tpch", "lineitem");
    Handle<Computation> myQ03CustomerSelection = makeObject<Q03CustomerSelection>("BUILDING");
    Handle<Computation> myQ03OrderSelection = makeObject<Q03OrderSelection>("1995-03-15");
    Handle<Computation> myQ03LineItemSelection = makeObject<Q03LineItemSelection>("1995-03-15");
    Handle<Computation> myQ03Join = makeObject<Q03Join>();
    Handle<Computation> myQ03Agg = makeObject<Q03Agg>();
    Handle<Computation> myQ03Writer = makeObject<WriteUserSet<Q03AggOut>> ("tpch", "q03_output_set");
    myQ03CustomerSelection->setInput(myCustomerScanner);
    myQ03OrderSelection->setInput(myOrderScanner);
    myQ03LineItemSelection->setInput(myLineItemScanner);
    myQ03Join->setInput(0, myQ03CustomerSelection);
    myQ03Join->setInput(1, myQ03OrderSelection);
    myQ03Join->setInput(2, myQ03LineItemSelection);
    myQ03Agg->setInput(myQ03Join);
    myQ03Writer->setInput(myQ03Agg);


    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery03", myQ03Writer)) {
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


    SetIterator<Q03AggOut> result =
            pdbClient.getSetIterator<Q03AggOut>("tpch", "q03_output_set");

    std::cout << "Query results: ";
    int count = 0;
    for (auto a : result) {
        double r = a->getValue();
        std::cout << "r=" << r << std::endl;
        count++;
    }
    std::cout << "Output count:" << count << "\n";
    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q03_output_set", errMsg)) {
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
