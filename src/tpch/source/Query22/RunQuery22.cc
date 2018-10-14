#ifndef RUN_QUERY22_CC
#define RUN_QUERY22_CC

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
#include "Query22.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>


using namespace std;
using namespace tpch;

/*
    val sub2 = udf { (x: String) => x.substring(0, 2) }
    val phone = udf { (x: String) => x.matches("13|31|23|29|30|18|17") }
    val isNull = udf { (x: Any) => println(x); true }

    val fcustomer = customer.select($"c_acctbal", $"c_custkey", sub2($"c_phone").as("cntrycode"))
      .filter(phone($"cntrycode"))

    val avg_customer = fcustomer.filter($"c_acctbal" > 0.0)
      .agg(avg($"c_acctbal").as("avg_acctbal"))

    order.groupBy($"o_custkey")
      .agg($"o_custkey").select($"o_custkey")
      .join(fcustomer, $"o_custkey" === fcustomer("c_custkey"), "right_outer")
      //.filter("o_custkey is null")
      .filter($"o_custkey".isNull)
      .join(avg_customer)
      .filter($"c_acctbal" > $"avg_acctbal")
      .groupBy($"cntrycode")
      .agg(count($"c_acctbal"), sum($"c_acctbal"))
      .sort($"cntrycode")
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
        pdbClient.registerType("libraries/libQ22AggregatedCntryBal.so", errMsg);
        pdbClient.registerType("libraries/libQ22CustomerAccbalAvg.so", errMsg);
        pdbClient.registerType("libraries/libQ22OrderCountSelection.so", errMsg);
        pdbClient.registerType("libraries/libQ22CntryBalAgg.so", errMsg);
        pdbClient.registerType("libraries/libQ22JoinedCntryBal.so", errMsg);
        pdbClient.registerType("libraries/libQ22CntryBalJoin.so", errMsg);
        pdbClient.registerType("libraries/libQ22OrderCountPerCustomer.so", errMsg);
    }    


    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q22_output_set", errMsg);
    if (!pdbClient.createSet<DoubleSumResult>(
            "tpch", "q22_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    // make the query graph
    Handle<Computation> myCustomerScanner = makeObject<ScanUserSet<Customer>>("tpch", "customer");
    Handle<Computation> myOrderScanner = makeObject<ScanUserSet<Order>>("tpch", "order");
    Handle<Computation> myQ22CustomerAccbalAvg = makeObject<Q22CustomerAccbalAvg>("13", "31", "23", "29", "30", "18", "17");
    Handle<Computation> myQ22OrderCountPerCustomer = makeObject<Q22OrderCountPerCustomer>();
    Handle<Computation> myQ22OrderCountSelection = 
         makeObject<Q22OrderCountSelection> ();
    Handle<Computation> myQ22CntryBalJoin = makeObject<Q22CntryBalJoin>();
    Handle<Computation> myQ22CntryBalAgg = makeObject<Q22CntryBalAgg>();
    Handle<Computation> myWriteSet = makeObject<WriteUserSet<Q22AggregatedCntryBal>>("tpch", "q22_output_set");

    myQ22CustomerAccbalAvg->setInput(myCustomerScanner);
    myQ22OrderCountPerCustomer->setInput(myOrderScanner);
    myQ22OrderCountSelection->setInput(myQ22OrderCountPerCustomer);
    myQ22CntryBalJoin->setInput(0, myQ22OrderCountSelection);
    myQ22CntryBalJoin->setInput(1, myCustomerScanner);
    myQ22CntryBalJoin->setInput(2, myQ22CustomerAccbalAvg);
    myQ22CntryBalAgg->setInput(myQ22CntryBalJoin);
    myWriteSet->setInput(myQ22CntryBalAgg);

    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery22", myWriteSet)) {
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


    SetIterator<Q22AggregatedCntryBal> result =
            pdbClient.getSetIterator<Q22AggregatedCntryBal>("tpch", "q22_output_set");

    std::cout << "Query results: ";
    int count = 0;
    for (auto a : result) {
        Q22AggregatedCntryBal r = *a;
        std::cout << r.cntrycode << ":" << r.avg.total << "," << r.avg.count << std::endl;
        count++;
    }
    std::cout << "Output count:" << count << "\n";
    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q22_output_set", errMsg)) {
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
