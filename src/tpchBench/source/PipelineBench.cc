#ifndef PIPELINE_BENCH_CC
#define PIPELINE_BENCH_CC

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
#include "SupplierData.h"

#include "Part.h"
#include "Supplier.h"
#include "LineItem.h"
#include "Order.h"
#include "Customer.h"
#include "CustomerStringSelection.h"
#include "CustomerStringSelectionVirtual.h"
#include "CustomerStringSelectionNot.h"
#include "CustomerStringSelectionVirtualNot.h"
#include "CustomerIntegerSelection.h"
#include "CustomerIntegerSelectionVirtual.h"
#include "CustomerIntegerSelectionNot.h"
#include "CustomerIntegerSelectionVirtualNot.h"
#include "Handle.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "SelectionComp.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ScanUserSet.h"
#include "WriteUserSet.h"
#include "ComputePlan.h"

#include "QueryOutput.h"
#include "DataTypes.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

using namespace std;



#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

int main(int argc, char* argv[]) {

    if (argc < 6) { 
        std::cout << "[Usage] #SelectionType (String/Integer) #SelectionRatio (low/high) #virtualOrNot (Y/N)  #batchSize #printResultOrNot (Y/N)" << std::endl;
    }


    std::string SelectionType = argv[1];
    std::string SelectionRatio = argv[2];

    bool virtualOrNot = false;
    if (strcmp(argv[3], "Y")==0) {
        virtualOrNot = true;
    }

    int batchSize = atoi(argv[4]);

    bool printResult = false;
    if (strcmp(argv[5], "Y")==0) {
        printResult = true;
    }

    string errMsg;

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

    // now, create the sets for storing Customer Data
    if (!pdbClient.createSet<Customer>(
            "TPCH_db", "t_output_set_1", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }

    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the query graph
    Handle<Computation> myScanSet = makeObject<ScanUserSet<Customer>>("TPCH_db", "tpch_bench_set1");
    myScanSet->setBatchSize(batchSize);

    Handle<Computation> mySelection = nullptr;

    if ((SelectionType == "String") && (SelectionRatio == "low") && (virtualOrNot == false)) {
        mySelection = makeObject<CustomerStringSelection>();
    } else if ((SelectionType == "String") && (SelectionRatio == "low") && (virtualOrNot == true)) {
        mySelection = makeObject<CustomerStringSelectionVirtual>();
    } else if ((SelectionType == "String") && (SelectionRatio == "high") && (virtualOrNot == false)) {
        mySelection = makeObject<CustomerStringSelectionNot>();
    } else if ((SelectionType == "String") && (SelectionRatio == "high") && (virtualOrNot == true)) {
        mySelection = makeObject<CustomerStringSelectionVirtualNot>();
    } else if ((SelectionType == "Integer") && (SelectionRatio == "low") && (virtualOrNot == false)) {
        mySelection = makeObject<CustomerIntegerSelection>();
    } else if ((SelectionType == "Integer") && (SelectionRatio == "low") && (virtualOrNot == true)) {
        mySelection = makeObject<CustomerIntegerSelectionVirtual>();
    } else if ((SelectionType == "Integer") && (SelectionRatio == "high") && (virtualOrNot == false)) {
        mySelection = makeObject<CustomerIntegerSelectionNot>();
    } else if ((SelectionType == "Integer") && (SelectionRatio == "high") && (virtualOrNot == true)) {
        mySelection = makeObject<CustomerIntegerSelectionVirtualNot>();
    } else {
        std::cout << "error in program inputs" << std::endl;
        std::cout << "[Usage] #SelectionType (String/Integer) #SelectionRatio (low/high) #virtualOrNot (Y/N)  #batchSize" << std::endl;
        exit(1);
    }

    mySelection->setInput(myScanSet);

    Handle<Computation> myWriteSet = makeObject<WriteUserSet<Customer>>("TPCH_db", "t_output_set_1");
    myWriteSet->setInput(mySelection);

    // Query Execution and Time Calculation

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.executeComputations(errMsg, myWriteSet)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }

    std::cout << std::endl;
    auto end = std::chrono::high_resolution_clock::now();

    float timeDifference =
        (float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count())) /
        (float)1000000000;

    std::cout << "#TimeDuration: " << timeDifference << " Second " << std::endl;

    if (printResult == true) {

        // Printing results to double check
        std::cout << "to print result..." << std::endl;


        SetIterator<Customer> result =
            pdbClient.getSetIterator<Customer>("TPCH_db", "t_output_set_1");

        std::cout << "Query results: ";
        int count = 0;
        for (auto a : result) {
            count++;
        }
        std::cout << "Output count:" << count << "\n";

    }
    // Remove the output set
    if (!pdbClient.removeSet("TPCH_db", "t_output_set_1", errMsg)) {
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

    const UseTemporaryAllocationBlock tempBlock2{1024 * 1024 * 128};
}
#endif
