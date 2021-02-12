#ifndef TPCH_GEN_TRACE_CC
#define TPCH_GEN_TRACE_CC

#include "CatalogClient.h"

#include <fstream>
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
#include <sqlite3.h>

#include "LambdaIdentifier.h"
#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "DataTypes.h"
#include "InterfaceFunctions.h"
#include "TPCHSchema.h"
#include "Handle.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "SelectionComp.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"
#include "QueryOutput.h"
#include "DataTypes.h"
#include "WriteUserSet.h"
#include "ScanUserSet.h"
#include "Query01.h"
#include "Query02.h"
#include "Query03.h"
#include "Query04.h"
#include "Query06.h"
#include "Query12.h"
#include "Query13.h"
#include "Query14.h"
#include "Query17.h"
#include "Query22.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

using namespace pdb;
using namespace tpch;

#define NUM_TPCH_QUERIES 3
#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

#define BLOCKSIZE (64 * MB)

float runQuery01(PDBClient & pdbClient, bool printResult = false) {

    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q01_output_set", errMsg);
    if (!pdbClient.createSet<Q01AggOut>(
            "tpch", "q01_output_set", errMsg)) {
        cout << errMsg << std::endl;
    } else {
        cout << "Created set.\n";
    }
    // make the query graph
    Handle<Computation> myLineItemScanner = makeObject<ScanUserSet<LineItem>>("tpch", "lineitem");
    Handle<Computation> myQ01Agg = makeObject<Q01Agg>();
    Handle<Computation> myQ01Writer = makeObject<WriteUserSet<Q01AggOut>> ("tpch", "q01_output_set");

    myQ01Agg->setInput(myLineItemScanner);
    myQ01Writer->setInput(myQ01Agg);


    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery01", myQ01Writer)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }

    std::cout << std::endl;
    auto end = std::chrono::high_resolution_clock::now();

    float timeDifference =
        (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count())) /
        (float)1000000000;

    // Printing results to double check

    if (printResult) {
        std::cout << "to print result..." << std::endl;


        SetIterator<Q01AggOut> result =
            pdbClient.getSetIterator<Q01AggOut>("tpch", "q01_output_set");

        std::cout << "Query results: ";
        int count = 0;
        for (auto a : result) {
            Q01ValueClass r = a->getValue();
            std::cout << "sum_qty=" << r.sum_qty << ", sum_base_price=" << r.sum_base_price << ", sum_disc_price=" << r.sum_disc_price
               << ", sum_charge=" << r.sum_charge << ", sum_disc=" << r.sum_disc << ", avg_qty=" << r.getAvgQty()
               << ", avg_price=" << r.getAvgPrice() << ", avg_disc=" << r.getAvgDiscount() << std::endl;
            count++;
        }
        std::cout << "Output count:" << count << "\n";
    }

    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q01_output_set", errMsg)) {
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

    return timeDifference;

}

float runQuery02(PDBClient & pdbClient, bool printResult = false) {
    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q02_output_set", errMsg);
    if (!pdbClient.createSet<Q02MinCostSelectionOutput>(
            "tpch", "q02_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

    // make the query graph
    Handle<Computation> myNationScanner = makeObject<ScanUserSet<Nation>>("tpch", "nation");
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



    if (printResult) {
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
    }

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

    return timeDifference;

}

float runQuery03(PDBClient & pdbClient, bool printResult=false) {
    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q03_output_set", errMsg);
    if (!pdbClient.createSet<Q03AggOut>(
            "tpch", "q03_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }


    


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

    if (printResult) {

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

    }

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

    return timeDifference;

}

float runQuery04(PDBClient & pdbClient, bool printResult=false) {

    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q04_output_set", errMsg);
    if (!pdbClient.createSet<Q04AggOut>(
            "tpch", "q04_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

    // make the query graph
    Handle<Computation> myOrderScanner = makeObject<ScanUserSet<Order>>("tpch", "order");
    Handle<Computation> myLineItemScanner = makeObject<ScanUserSet<LineItem>>("tpch", "lineitem");
    Handle<Computation> myQ04OrderSelection = makeObject<Q04OrderSelection>();
    Handle<Computation> myQ04Join = makeObject<Q04Join>();
    Handle<Computation> myQ04Agg = makeObject<Q04Agg>();
    Handle<Computation> myQ04Writer = makeObject<WriteUserSet<Q04AggOut>> ("tpch", "q04_output_set");
    myQ04OrderSelection->setInput(myOrderScanner);
    myQ04Join->setInput(0, myQ04OrderSelection);
    myQ04Join->setInput(1, myLineItemScanner);
    myQ04Agg->setInput(myQ04Join);
    myQ04Writer->setInput(myQ04Agg);


    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery04", myQ04Writer)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }

    std::cout << std::endl;
    auto end = std::chrono::high_resolution_clock::now();

    float timeDifference =
        (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count())) /
        (float)1000000000;


    if (printResult) {

        // Printing results to double check
        std::cout << "to print result..." << std::endl;

        SetIterator<Q04AggOut> result =
            pdbClient.getSetIterator<Q04AggOut>("tpch", "q04_output_set");

        std::cout << "Query results: ";
        int count = 0;
        for (auto a : result) {
            std::cout << a->o_orderpriority << ":" << a->count << std::endl;
            count++;
        }
        std::cout << "Output count:" << count << "\n";

    }
    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q04_output_set", errMsg)) {
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

    return timeDifference;

}

float runQuery06(PDBClient & pdbClient, bool printResult=false) {

    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q06_output_set", errMsg);
    if (!pdbClient.createSet<DoubleSumResult>(
            "tpch", "q06_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }
    // make the query graph
    Handle<Computation> myLineItemScanner = makeObject<ScanUserSet<LineItem>>("tpch", "lineitem");
    Handle<Computation> myQ06LineItemSelection = makeObject<Q06LineItemSelection>();
    Handle<Computation> myQ06Agg = makeObject<Q06Agg>();
    Handle<Computation> myQ06Writer = makeObject<WriteUserSet<DoubleSumResult>> ("tpch", "q06_output_set");
    myQ06LineItemSelection->setInput(myLineItemScanner);
    myQ06Agg->setInput(myQ06LineItemSelection);
    myQ06Writer->setInput(myQ06Agg);

    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery06", myQ06Writer)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }

    std::cout << std::endl;
    auto end = std::chrono::high_resolution_clock::now();

    float timeDifference =
        (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count())) /
        (float)1000000000;


    if (printResult) {

       // Printing results to double check
       std::cout << "to print result..." << std::endl;


       SetIterator<DoubleSumResult> result =
            pdbClient.getSetIterator<DoubleSumResult>("tpch", "q06_output_set");

       std::cout << "Query results: ";
       int count = 0;
       for (auto a : result) {
           std::cout << a->total << std::endl;
           count++;
       }
       std::cout << "Output count:" << count << "\n";
    }

    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q06_output_set", errMsg)) {
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

    return timeDifference;


}

float runQuery12(PDBClient & pdbClient, bool printResult=false) {

    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q12_output_set", errMsg);
    if (!pdbClient.createSet<Q12AggOut>(
            "tpch", "q12_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }
    // make the query graph
    Handle<Computation> myOrderScanner = makeObject<ScanUserSet<Order>>("tpch", "order");
    Handle<Computation> myLineItemScanner = makeObject<ScanUserSet<LineItem>>("tpch", "lineitem");
    Handle<Computation> myQ12LineItemSelection = makeObject<Q12LineItemSelection>();
    Handle<Computation> myQ12Join = makeObject<Q12Join>();
    Handle<Computation> myQ12Agg = makeObject<Q12Agg>();
    Handle<Computation> myQ12Writer = makeObject<WriteUserSet<Q12AggOut>> ("tpch", "q12_output_set");
    myQ12LineItemSelection->setInput(myLineItemScanner);
    myQ12Join->setInput(0, myOrderScanner);
    myQ12Join->setInput(1, myQ12LineItemSelection);
    myQ12Agg->setInput(myQ12Join);
    myQ12Writer->setInput(myQ12Agg);

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery12", myQ12Writer)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }

    std::cout << std::endl;
    auto end = std::chrono::high_resolution_clock::now();

    float timeDifference =
        (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count())) /
        (float)1000000000;


    if (printResult) {
         // Printing results to double check
         std::cout << "to print result..." << std::endl;


         SetIterator<Q12AggOut> result =
            pdbClient.getSetIterator<Q12AggOut>("tpch", "q12_output_set");

         std::cout << "Query results: ";
         int count = 0;
         for (auto a : result) {
             std::cout << a->l_shipmode << ":" << a->value.high_line_count << "," << a->value.low_line_count << std::endl;
             count++;
         }
         std::cout << "Output count:" << count << "\n";
    }
    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q12_output_set", errMsg)) {
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

    return timeDifference;

}

float runQuery13(PDBClient & pdbClient, bool printResult=false) {
    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q13_output_set", errMsg);
    if (!pdbClient.createSet<Q13CountResult>(
            "tpch", "q13_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }
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


    if (printResult) {

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
    }
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

    return timeDifference;

}

float runQuery14(PDBClient & pdbClient, bool printResult=false) {
    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q14_output_set", errMsg);
    if (!pdbClient.createSet<Q14AggOut>(
            "tpch", "q14_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }
    // make the query graph
    Handle<Computation> myPartScanner = makeObject<ScanUserSet<Part>>("tpch", "part");
    Handle<Computation> myLineItemScanner = makeObject<ScanUserSet<LineItem>>("tpch", "lineitem");
    Handle<Computation> myQ14LineItemSelection = makeObject<Q14LineItemSelection>();
    Handle<Computation> myQ14Join = makeObject<Q14Join>();
    Handle<Computation> myQ14Agg = makeObject<Q14Agg>();
    Handle<Computation> myQ14Writer = makeObject<WriteUserSet<Q14AggOut>> ("tpch", "q14_output_set");
    myQ14LineItemSelection->setInput(myLineItemScanner);
    myQ14Join->setInput(0, myQ14LineItemSelection);
    myQ14Join->setInput(1, myPartScanner);
    myQ14Agg->setInput(myQ14Join);
    myQ14Writer->setInput(myQ14Agg);

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.queryClient.executeComputations(errMsg, "TPCHQuery14", myQ14Writer)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }

    std::cout << std::endl;
    auto end = std::chrono::high_resolution_clock::now();

    float timeDifference =
        (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count())) /
        (float)1000000000;

    if (printResult) {

        // Printing results to double check
        std::cout << "to print result..." << std::endl;


        SetIterator<Q14AggOut> result =
            pdbClient.getSetIterator<Q14AggOut>("tpch", "q14_output_set");

        std::cout << "Query results: ";
        int count = 0;
        for (auto a : result) {
            std::cout << a->value.getPromoRevenue() << std::endl;
            count++;
        }
        std::cout << "Output count:" << count << "\n";
    }
    std::cout << "#TimeDuration for query execution: " << timeDifference << " Second " << std::endl;

    // Remove the output set
    if (!pdbClient.removeSet("tpch", "q14_output_set", errMsg)) {
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

    return timeDifference;
}

float runQuery17(PDBClient & pdbClient, bool printResult=false) {

    // now, create the sets for storing Customer Data
    std::string errMsg;
    pdbClient.removeSet("tpch", "q17_output_set", errMsg);
    if (!pdbClient.createSet<DoubleSumResult>(
            "tpch", "q17_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

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


    if (printResult) {

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
    }
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

    return timeDifference;

}

float runQuery22(PDBClient & pdbClient, bool printResult=false) {

    std::string errMsg;
    pdbClient.removeSet("tpch", "q22_output_set", errMsg);
    if (!pdbClient.createSet<DoubleSumResult>(
            "tpch", "q22_output_set", errMsg)) {
        cout << errMsg << endl;
    } else {
        cout << "Created set.\n";
    }

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


    if (printResult) {
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
    }
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

    return timeDifference;

}

// A function to parse a Line
std::vector<std::string> parseLine(std::string line) {
    stringstream lineStream(line);
    std::vector<std::string> tokens;
    string token;
    while (getline(lineStream, token, '|')) {
        tokens.push_back(token);
    }
    return tokens;
}

//A function to create an object based on dataType and text
Handle<Object>  createObject(std::string line, std::string dataType) {
   
     
    std::vector<std::string> tokens = parseLine(line);
    Handle<Object> objectToAdd = nullptr;
    if (dataType == "Customer") {
        objectToAdd = makeObject<tpch::Customer>(atoi(tokens.at(0).c_str()),
                                                        tokens.at(1),
                                                        tokens.at(2),
                                                        atoi(tokens.at(3).c_str()),
                                                        tokens.at(4),
                                                        atof(tokens.at(5).c_str()),
                                                        tokens.at(6),
                                                        tokens.at(7));
    } else if (dataType == "LineItem") {
       objectToAdd = makeObject<LineItem>(atoi(tokens.at(0).c_str()),
                                                        atoi(tokens.at(1).c_str()),
                                                        atoi(tokens.at(2).c_str()),
                                                        atoi(tokens.at(3).c_str()),
                                                        atof(tokens.at(4).c_str()),
                                                        atof(tokens.at(5).c_str()),
                                                        atof(tokens.at(6).c_str()),
                                                        atof(tokens.at(7).c_str()),
                                                        tokens.at(8),
                                                        tokens.at(9),
                                                        tokens.at(10),
                                                        tokens.at(11),
                                                        tokens.at(12),
                                                        tokens.at(13),
                                                        tokens.at(14),
                                                        tokens.at(15));

    } else if (dataType == "Nation") {
       objectToAdd = makeObject<Nation>(atoi(tokens.at(0).c_str()),
                                                        tokens.at(1),
                                                        atoi(tokens.at(2).c_str()),
                                                        tokens.at(3));

    } else if (dataType == "Order") {
       objectToAdd = makeObject<Order>(atoi(tokens.at(0).c_str()),
                                                        atoi(tokens.at(1).c_str()),
                                                        tokens.at(2),
                                                        atof(tokens.at(3).c_str()),
                                                        tokens.at(4),
                                                        tokens.at(5),
                                                        tokens.at(6),
                                                        atoi(tokens.at(7).c_str()),
                                                        tokens.at(8));
    } else if (dataType == "Part") {
       objectToAdd = makeObject<Part>(atoi(tokens.at(0).c_str()),
                                                        tokens.at(1),
                                                        tokens.at(2),
                                                        tokens.at(3),
                                                        tokens.at(4),
                                                        atoi(tokens.at(5).c_str()),
                                                        tokens.at(6),
                                                        atof(tokens.at(7).c_str()),
                                                        tokens.at(8));
    } else if (dataType == "PartSupp") {
       objectToAdd = makeObject<PartSupp>(atoi(tokens.at(0).c_str()),
                                                        atoi(tokens.at(1).c_str()),
                                                        atoi(tokens.at(2).c_str()),
                                                        atof(tokens.at(3).c_str()),
                                                        tokens.at(4));

    } else if (dataType == "Region" ) {
       objectToAdd = makeObject<Region>(atoi(tokens.at(0).c_str()),
                                                        tokens.at(1),
                                                        tokens.at(2));
    } else if (dataType == "Supplier" ) {
       objectToAdd = makeObject<Supplier>(atoi(tokens.at(0).c_str()),
                                                        tokens.at(1),
                                                        tokens.at(2),
                                                        atoi(tokens.at(3).c_str()),
                                                        tokens.at(4),
                                                        atof(tokens.at(5).c_str()),
                                                        tokens.at(6));
    }
    return objectToAdd;
}


void removeSet (PDBClient & pdbClient, std::string dataType) {

    std::string errMsg;
    
    if (dataType == "Customer") {
         pdbClient.removeSet("tpch", "customer", errMsg);
    } else if (dataType == "LineItem") {
         pdbClient.removeSet("tpch", "lineitem", errMsg);
    } else if (dataType == "Nation") {
         pdbClient.removeSet("tpch", "nation", errMsg);
    } else if (dataType == "Order") {
         pdbClient.removeSet("tpch", "order", errMsg);
    } else if (dataType == "Part") {
         pdbClient.removeSet("tpch", "part", errMsg);
    } else if (dataType == "PartSupp") {
         pdbClient.removeSet("tpch", "partsupp", errMsg);
    } else if (dataType == "Region") {
         pdbClient.removeSet("tpch", "region", errMsg);
    } else if (dataType == "Supplier") {
         pdbClient.removeSet("tpch", "supplier", errMsg);
    }
}


void createSet (PDBClient & pdbClient, std::string dataType, Handle<LambdaIdentifier> partitionLambda) {

    std::string errMsg;
    pdbClient.createDatabase("tpch", errMsg);

    if (dataType == "Customer") {
         pdbClient.removeSet("tpch", "customer", errMsg);
         std::cout << "to create set for Customer" << std::endl;
         bool customerRes = pdbClient.createSet<Customer>("tpch", "customer", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadCustomer", nullptr, partitionLambda);
    } else if (dataType == "LineItem") {
         pdbClient.removeSet("tpch", "lineitem", errMsg);
         std::cout << "to create set for LineItem" << std::endl;
         bool lineitemRes = pdbClient.createSet<LineItem>("tpch", "lineitem", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadLineItem", nullptr, partitionLambda);
    } else if (dataType == "Nation") {
         pdbClient.removeSet("tpch", "nation", errMsg);
         std::cout << "to create set for Nation" << std::endl;
         bool nationRes = pdbClient.createSet<Nation>("tpch", "nation", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadNation", nullptr, partitionLambda);
    } else if (dataType == "Order") {
         pdbClient.removeSet("tpch", "order", errMsg);
         std::cout << "to create set for Order" << std::endl;
         bool orderRes = pdbClient.createSet<Order>("tpch", "order", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadOrder", nullptr, partitionLambda);
    } else if (dataType == "Part") {
         pdbClient.removeSet("tpch", "part", errMsg);
         std::cout << "to create set for Part" << std::endl;
         bool partRes = pdbClient.createSet<Part>("tpch", "part", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadPart", nullptr, partitionLambda);
    } else if (dataType == "PartSupp") {
         pdbClient.removeSet("tpch", "partsupp", errMsg);
         std::cout << "to create set for PartSupp" << std::endl;
         bool partsuppRes = pdbClient.createSet<PartSupp>("tpch", "partsupp", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadPartSupp", nullptr, partitionLambda);
    } else if (dataType == "Region") {
         pdbClient.removeSet("tpch", "region", errMsg);
         std::cout << "to create set for Region" << std::endl;
         bool regionRes = pdbClient.createSet<Region>("tpch", "region", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadRegion", nullptr, partitionLambda);
    } else if (dataType == "Supplier") {
         pdbClient.removeSet("tpch", "supplier", errMsg);
         std::cout << "to create set for Supplier" << std::endl;
         bool supplierRes = pdbClient.createSet<Supplier>("tpch", "supplier", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadSupplier", nullptr, partitionLambda);
    }

}


void sendData (PDBClient & pdbClient, Handle<Vector<Handle<Object>>> objects, std::string dataType) {
    std::cout << "to send vector with " << objects->size() << std::endl;
    std::string errMsg;
    if (dataType == "Customer") {
        pdbClient.sendData<Object> (
        std::pair<std::string, std::string>("customer", "tpch"), objects, errMsg);
    } else if (dataType == "LineItem") {
        pdbClient.sendData<Object> (
        std::pair<std::string, std::string>("lineitem", "tpch"), objects, errMsg);
    } else if (dataType == "Nation") {
        pdbClient.sendData<Object> (
        std::pair<std::string, std::string>("nation", "tpch"), objects, errMsg);
    } else if (dataType == "Order") {
        pdbClient.sendData<Object> (
        std::pair<std::string, std::string>("order", "tpch"), objects, errMsg);
    } else if (dataType == "Part") {
        pdbClient.sendData<Object> (
        std::pair<std::string, std::string>("part", "tpch"), objects, errMsg);
    } else if (dataType == "PartSupp") {
        pdbClient.sendData<Object> (
        std::pair<std::string, std::string>("partsupp", "tpch"), objects, errMsg);
    } else if (dataType == "Region") {
        pdbClient.sendData<Object> (
        std::pair<std::string, std::string>("region", "tpch"), objects, errMsg);
    } else if (dataType == "Supplier") {
        pdbClient.sendData<Object> (
        std::pair<std::string, std::string>("supplier", "tpch"), objects, errMsg);
    }

}


void loadData(PDBClient & pdbClient, std::string fileName, std::string dataType) {


    std::cout << "to load data from " << fileName << " for type " << dataType << std::endl;
    std::string line;
    std::string delimiter = "|";
    std::ifstream infile;
    bool rollback = false;
    bool end = false;
    int numObjects = 0;
    infile.open(fileName.c_str());

    //different types
    Handle<Vector<Handle<Customer>>> customers = nullptr; 
    Handle<Vector<Handle<LineItem>>> lineitems = nullptr;
    Handle<Vector<Handle<Nation>>> nations = nullptr;
    Handle<Vector<Handle<Order>>> orders = nullptr;
    Handle<Vector<Handle<Part>>> parts = nullptr;
    Handle<Vector<Handle<PartSupp>>> partsupps = nullptr;
    Handle<Vector<Handle<Region>>> regions = nullptr;
    Handle<Vector<Handle<Supplier>>> suppliers = nullptr;
    Handle<Vector<Handle<Object>>> objects = nullptr;
    while (!end) {
        makeObjectAllocatorBlock((size_t)BLOCKSIZE, true);
        if (dataType == "Customer") {
            customers = makeObject<Vector<Handle<Customer>>>();
            objects = unsafeCast<Vector<Handle<Object>>, Vector<Handle<Customer>>> (customers);
        } else if (dataType == "LineItem") {
            lineitems = makeObject<Vector<Handle<LineItem>>>();
            objects = unsafeCast<Vector<Handle<Object>>, Vector<Handle<LineItem>>> (lineitems);
        } else if (dataType == "Nation") {
            nations = makeObject<Vector<Handle<Nation>>>();
            objects = unsafeCast<Vector<Handle<Object>>, Vector<Handle<Nation>>> (nations);
        } else if (dataType == "Order") {
            orders = makeObject<Vector<Handle<Order>>>();
            objects = unsafeCast<Vector<Handle<Object>>, Vector<Handle<Order>>> (orders);
        } else if (dataType == "Part") {
            parts = makeObject<Vector<Handle<Part>>>();
            objects = unsafeCast<Vector<Handle<Object>>, Vector<Handle<Part>>> (parts);
        } else if (dataType == "PartSupp") {
            partsupps = makeObject<Vector<Handle<PartSupp>>>();
            objects = unsafeCast<Vector<Handle<Object>>, Vector<Handle<PartSupp>>> (partsupps);
        } else if (dataType == "Region") {
            regions = makeObject<Vector<Handle<Region>>>();
            objects = unsafeCast<Vector<Handle<Object>>, Vector<Handle<Region>>> (regions);
        } else if (dataType == "Supplier") {
            suppliers = makeObject<Vector<Handle<Supplier>>>();
            objects = unsafeCast<Vector<Handle<Object>>, Vector<Handle<Supplier>>> (suppliers);
        }
        
        
        try {

            while (1) {
                if (!rollback) {
                    if (!std::getline(infile, line)) {
                         end = true;
                         break;
                    } else {
                         Handle<Object> objectToAdd = createObject(line, dataType);
                         objects->push_back(objectToAdd);
                         numObjects++;
                    }
                } else {
                    rollback = false;
                    Handle<Object> objectToAdd = createObject(line, dataType);
                    objects->push_back(objectToAdd);
                    numObjects++;
                }
            }
            sendData(pdbClient, objects, dataType); 

        } catch (NotEnoughSpace & n) {

            sendData(pdbClient, objects, dataType);
            rollback = true;

        }

    }
    std::cout << "sent " << numObjects << " " << dataType << " objects" << std::endl; 
    infile.close();
}




static sqlite3 * selfLearningDBHandler;


bool openDB() {

    std::string pathToDB = "selfLearningDB/dbFile";
    if (sqlite3_open_v2(pathToDB.c_str(), &selfLearningDBHandler,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL) == SQLITE_OK) {

        sqlite3_exec(selfLearningDBHandler, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
        std::cout << "self learning database open successfully" << std::endl;
        return true;
    } else {
        std::cout << "failure in opening self learning database" << std::endl;
        return false;
    }
    

}

bool execDB(std::string cmdString) {

    std::cout << "command: " << cmdString << std::endl;
    sqlite3_stmt * cmdStatement;
    if (sqlite3_prepare_v2 (selfLearningDBHandler, cmdString.c_str(), -1, &cmdStatement,
                     NULL) == SQLITE_OK) {
        sqlite3_step(cmdStatement);
        sqlite3_finalize(cmdStatement);
        return true;
    } else {
        std::string error = sqlite3_errmsg(selfLearningDBHandler);
        std::cout << error << std::endl;
        sqlite3_finalize(cmdStatement);
        return false;
    }

}



Handle<LambdaIdentifier> getLambdaIdentifier(int lambdaId){
    if (lambdaId == -1) {
        return nullptr;
    }          
    std::string jobName = "";
    std::string computationName = "";
    std::string lambdaName = "";

    std::string queryString = "SELECT JOB_NAME, COMPUTATION_NAME, LAMBDA_NAME "
                               "FROM LAMBDA_STAT WHERE ID = " + std::to_string(lambdaId);
    std::cout << "to get lambda identifier: " << queryString << std::endl;
    sqlite3_stmt * statement;
    int res;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
               NULL) == SQLITE_OK) {
        if ((res = sqlite3_step(statement)) == SQLITE_ROW) {
            jobName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
            computationName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)); 
            lambdaName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
            std::cout << "jobName is " << jobName << ", computationName is " << computationName 
                      << ", lambdaName is " << lambdaName << std::endl;
            return makeObject<LambdaIdentifier> (jobName, computationName, lambdaName);
        } 
    } else {
            std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
            sqlite3_finalize(statement);
            return nullptr;
    }
    sqlite3_finalize(statement);
    return nullptr;   
}

typedef struct PartitionSchemeStruct {
    int partitionSchemeId = 0;
    int customerLambdaId = -1;
    int lineitemLambdaId = -1;
    int orderLambdaId = -1;
    int partLambdaId = -1;
    int supplierLambdaId = -1;
    int partsuppLambdaId = -1;
    int regionLambdaId = -1;
    int nationLambdaId = -1;
} PartitionSchemeStruct_t;


int main(int argc, char* argv[]) {

    std::string filePath="";
    int startingId = 0;
    int endingId = 720;
    if (argc > 1) {
        filePath = argv[1];
    }
    if (argc > 2) {
        startingId = atoi(argv[2]);
    }
    if (argc > 3) {
        endingId = atoi(argv[3]);
    }
    if ((argc > 4) || (argc <= 1)) {
       std::cout << "Usage: #tpchFilePath #startingId #endingId" << std::endl;
       exit(1);
    }
     
    //prerequisite: run all queries once then run bin/tpchPrepareTraining

    // Connection info
    string masterHostname = "localhost";
    int masterPort = 8108;

    // register the shared employee class
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");

    pdb::PDBClient pdbClient(
            masterPort, masterHostname, clientLogger, false, true);

    CatalogClient catalogClient(
            masterPort,
            masterHostname,
            clientLogger);

    // to open database
    if (openDB() == false) {
        return false;
    }

    // open script file to log the sqlite commands for inserting tracing data
    ofstream outputFile;
    outputFile.open("gen_trace.sql");
    


    //then we create a RunStatistics table for storing the statistics for each run of query
    // --id, the id of query run
    // --jobName, the id of the query
    // --partitionSchemeId, the id of the partition scheme
    // --environmentId, the id of the environment
    // --latency
    // --throughput

    //execDB ("DROP TABLE IF EXISTS RUN_STAT;");    
    execDB ("CREATE TABLE IF NOT EXISTS RUN_STAT (ID BIGINT PRIMARY KEY, "
            "JOB_NAME VARCHAR(128), PARTITION_SCHEME_ID BIGINT, "
            "ENVIRONMENT_ID BIGINT, LATENCY FLOAT) WITHOUT ROWID;");

    const UseTemporaryAllocationBlock tempBlock{(size_t)4 * (size_t)1024 * (size_t)1024 * (size_t)1024};
 
    //then in a loop, for each partition scheme,
    //we create, load and partition each table based on the lambda for that table in the partition scheme
    //we run each query, and for each run we update the RunStatistics table
    std::vector<PartitionSchemeStruct_t> schemes;
    std::string queryString = "SELECT ID, CUSTOMER_LAMBDA_ID, LINEITEM_LAMBDA_ID, ORDER_LAMBDA_ID, "
                  "PART_LAMBDA_ID, SUPPLIER_LAMBDA_ID, PARTSUPP_LAMBDA_ID, "
                  "REGION_LAMBDA_ID, NATION_LAMBDA_ID "
                  "FROM PARTITION_SCHEME_STAT WHERE ID >= " + std::to_string(startingId) +
                  " AND ID <= " + std::to_string(endingId);
    sqlite3_stmt * statement;
    int res;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
               NULL) == SQLITE_OK) {
       while ((res = sqlite3_step(statement)) == SQLITE_ROW) {
           PartitionSchemeStruct_t curScheme;
           curScheme.partitionSchemeId = sqlite3_column_int64(statement, 0);
           curScheme.customerLambdaId = sqlite3_column_int(statement, 1);
           curScheme.lineitemLambdaId = sqlite3_column_int(statement, 2);
           curScheme.orderLambdaId = sqlite3_column_int(statement, 3);
           curScheme.partLambdaId = sqlite3_column_int(statement, 4);
           curScheme.supplierLambdaId = sqlite3_column_int(statement, 5);
           curScheme.partsuppLambdaId = sqlite3_column_int(statement, 6);
           curScheme.regionLambdaId= sqlite3_column_int(statement, 7);
           curScheme.nationLambdaId = sqlite3_column_int(statement, 8);
           schemes.push_back(curScheme);
           std::cout << curScheme.partitionSchemeId << ","
                     << curScheme.customerLambdaId << ","
                     << curScheme.lineitemLambdaId << ","
                     << curScheme.orderLambdaId << std::endl;
       }
    }  else {
           std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
           sqlite3_finalize(statement);
           exit(1);
    }
    sqlite3_finalize(statement);

    int curLambdaId;
    int runId = NUM_TPCH_QUERIES * startingId;//you need change 3 to the number of training queries
    int partitionSchemeId = 0;

    int customerLambdaId = -2;
    int lineitemLambdaId = -2;
    int orderLambdaId = -2;
    int partLambdaId = -2;
    int supplierLambdaId = -2;
    int partsuppLambdaId = -2;
    int regionLambdaId = -2;
    int nationLambdaId = -2;
    for (int i = 0; i < schemes.size(); i++) {
        partitionSchemeId = schemes[i].partitionSchemeId;
        if (customerLambdaId != (curLambdaId = schemes[i].customerLambdaId)) {
           customerLambdaId = curLambdaId;
           std::cout << "customerLambdaId = " << customerLambdaId << std::endl;
           //we need recreate the customer table
           removeSet(pdbClient, "Customer");           
           createSet(pdbClient, "Customer", getLambdaIdentifier(customerLambdaId));
           loadData(pdbClient, filePath+"/customer.tbl", "Customer");  
        }
        if (lineitemLambdaId != (curLambdaId = schemes[i].lineitemLambdaId)) {
           lineitemLambdaId = curLambdaId;
           std::cout << "lineitemLambdaId = " << lineitemLambdaId << std::endl;
           //we need recreate the lineitem table 
           removeSet(pdbClient, "LineItem");
           createSet(pdbClient, "LineItem", getLambdaIdentifier(lineitemLambdaId));
           loadData(pdbClient, filePath+"/lineitem.tbl", "LineItem");
        }
        if (orderLambdaId != (curLambdaId = schemes[i].orderLambdaId)) {
           orderLambdaId = curLambdaId;
           std::cout << "orderLambdaId = " << orderLambdaId << std::endl;
           //we need recreate the order table
           removeSet(pdbClient, "Order");
           createSet(pdbClient, "Order", getLambdaIdentifier(orderLambdaId));
           loadData(pdbClient, filePath+"/orders.tbl", "Order");
        }
        if (partLambdaId != (curLambdaId = schemes[i].partLambdaId)) {
           partLambdaId = curLambdaId;
           //we need recreate the part table
           removeSet(pdbClient, "Part");
           createSet(pdbClient, "Part", getLambdaIdentifier(partLambdaId));
           loadData(pdbClient, filePath+"/part.tbl", "Part");
        }      
        if (supplierLambdaId != (curLambdaId = schemes[i].supplierLambdaId)) {
           supplierLambdaId = curLambdaId;
           //we need recreate the supplier table
           removeSet(pdbClient, "Supplier");
           createSet(pdbClient, "Supplier", getLambdaIdentifier(supplierLambdaId));
           loadData(pdbClient, filePath+"/supplier.tbl", "Supplier");
        }
        if (partsuppLambdaId != (curLambdaId = schemes[i].partsuppLambdaId)) {
           partsuppLambdaId = curLambdaId;
           //we need recreate the partsupp table
           removeSet(pdbClient, "PartSupp");
           createSet(pdbClient, "PartSupp", getLambdaIdentifier(partsuppLambdaId));
           loadData(pdbClient, filePath+"/partsupp.tbl", "PartSupp");
        }
        if (regionLambdaId != (curLambdaId = schemes[i].regionLambdaId)) {
           regionLambdaId = curLambdaId;
           //we need recreate the region table
           removeSet(pdbClient, "Region");
           createSet(pdbClient, "Region", getLambdaIdentifier(regionLambdaId));
           loadData(pdbClient, filePath+"/region.tbl", "Region");
        }
        if (nationLambdaId != (curLambdaId = schemes[i].nationLambdaId)) {
           nationLambdaId = curLambdaId;
           //we need recreate the nation table
           removeSet(pdbClient, "Nation");
           createSet(pdbClient, "Nation", getLambdaIdentifier(nationLambdaId));
           loadData(pdbClient, filePath+"/nation.tbl", "Nation");
        }

        //run all queries, and for each query, we collect stats and insert to run_stat table;
        float curLatency;
        curLatency = runQuery04(pdbClient);
        std::string cmdString = "INSERT INTO RUN_STAT VALUES (" 
                + std::to_string(runId) + ",'TPCHQuery04',"
                + std::to_string(partitionSchemeId) + ",0,"
                + std::to_string(curLatency) +")";
        runId ++;

        curLatency = runQuery12(pdbClient);
        cmdString += ",(" + std::to_string(runId) + ",'TPCHQuery12',"
                          + std::to_string(partitionSchemeId) + ",0,"
                          + std::to_string(curLatency) +")";
        runId ++;

        curLatency = runQuery17(pdbClient);
        cmdString += ",(" + std::to_string(runId) + ",'TPCHQuery17',"
                          + std::to_string(partitionSchemeId) + ",0,"
                          + std::to_string(curLatency) +")";
        runId ++;

/*
        curLatency = runQuery04(pdbClient);
        cmdString += ",(" + std::to_string(runId) + ",'TPCHQuery04',"
                          + std::to_string(partitionSchemeId) + ",0,"
                          + std::to_string(curLatency) +")";
        runId ++;

        curLatency = runQuery06(pdbClient);
        cmdString += ",(" + std::to_string(runId) + ",'TPCHQuery06',"
                          + std::to_string(partitionSchemeId) + ",0,"
                          + std::to_string(curLatency) +")";
        runId ++;

        curLatency = runQuery12(pdbClient);
        cmdString += ",(" + std::to_string(runId) + ",'TPCHQuery12',"
                          + std::to_string(partitionSchemeId) + ",0,"
                          + std::to_string(curLatency) +")";
        runId ++;

        curLatency = runQuery13(pdbClient);
        cmdString += ",(" + std::to_string(runId) + ",'TPCHQuery13',"
                          + std::to_string(partitionSchemeId) + ",0,"
                          + std::to_string(curLatency) +")";
        runId ++;

        curLatency = runQuery14(pdbClient);
        cmdString += ",(" + std::to_string(runId) + ",'TPCHQuery14',"
                          + std::to_string(partitionSchemeId) + ",0,"
                          + std::to_string(curLatency) +")";
        runId ++;

        curLatency = runQuery17(pdbClient);
        cmdString += ",(" + std::to_string(runId) + ",'TPCHQuery17',"
                          + std::to_string(partitionSchemeId) + ",0,"
                          + std::to_string(curLatency) +")";
        runId ++;

        curLatency = runQuery22(pdbClient);
        cmdString += ",(" + std::to_string(runId) + ",'TPCHQuery22',"
                          + std::to_string(partitionSchemeId) + ",0,"
                          + std::to_string(curLatency) +")";
        runId ++;
*/
        cmdString+=";";
        std::cout << "insert RunStat for partitionSchemeId=" << partitionSchemeId << ":" << cmdString << std::endl;
        outputFile << cmdString << std::endl;
    }
        
    sqlite3_close_v2(selfLearningDBHandler);
    outputFile.close();
    system("sqlite3 selfLearningDB/dbFile < gen_trace.sql");
}

#endif
