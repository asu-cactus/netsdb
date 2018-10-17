#ifndef TPCH_REGISTER_LIBRARIES_CC
#define TPCH_REGISTER_LIBRARIES_CC

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

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

using namespace pdb;
using namespace tpch;



void registerLibraries (PDBClient & pdbClient) {

    std::string errMsg;
    pdbClient.registerType ("libraries/libTpchCustomer.so", errMsg);
    pdbClient.registerType ("libraries/libTpchLineItem.so", errMsg);
    pdbClient.registerType ("libraries/libTpchNation.so", errMsg);
    pdbClient.registerType ("libraries/libTpchOrder.so", errMsg);
    pdbClient.registerType ("libraries/libTpchPart.so", errMsg);
    pdbClient.registerType ("libraries/libTpchPartSupp.so", errMsg);
    pdbClient.registerType ("libraries/libTpchRegion.so", errMsg);
    pdbClient.registerType ("libraries/libTpchSupplier.so", errMsg);

    bool query01 = true;

    if (query01) {

       pdbClient.registerType ("libraries/libQ01Agg.so", errMsg);
       pdbClient.registerType ("libraries/libQ01AggOut.so", errMsg);
       pdbClient.registerType ("libraries/libQ01KeyClass.so", errMsg);
       pdbClient.registerType ("libraries/libQ01ValueClass.so", errMsg);

    }

    bool query02 = true;

    if (query02) {
   
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

    bool query03 = true;

    if (query03) {
        pdbClient.registerType ("libraries/libQ03Agg.so", errMsg);
        pdbClient.registerType ("libraries/libQ03AggOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ03Join.so", errMsg);
        pdbClient.registerType ("libraries/libQ03JoinOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ03CustomerSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ03OrderSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ03LineItemSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ03KeyClass.so", errMsg);
    }

    bool query04 = true;

    if (query04) {

        pdbClient.registerType ("libraries/libQ04Agg.so", errMsg);
        pdbClient.registerType ("libraries/libQ04AggOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ04Join.so", errMsg);
        pdbClient.registerType ("libraries/libQ04OrderSelection.so", errMsg);

    }

    bool query06 = true;

    if (query06) {

        pdbClient.registerType ("libraries/libQ06Agg.so", errMsg);
        pdbClient.registerType ("libraries/libQ06LineItemSelection.so", errMsg);

    }

    bool query12 = true;

    if (query12) {

        pdbClient.registerType ("libraries/libQ12LineItemSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ12JoinOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ12Join.so", errMsg);
        pdbClient.registerType ("libraries/libQ12ValueClass.so", errMsg);
        pdbClient.registerType ("libraries/libQ12AggOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ12Agg.so", errMsg);

    }


    bool query13 = true;
    
    if (query13) {

        pdbClient.registerType ("libraries/libQ13CountResult.so", errMsg);
        pdbClient.registerType ("libraries/libQ13CustomerOrderJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ13OrderSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ13CustomerDistribution.so", errMsg);
        pdbClient.registerType ("libraries/libQ13CustomerOrders.so", errMsg);
        pdbClient.registerType ("libraries/libQ13OrdersPerCustomer.so", errMsg);

    }

    bool query14 = true;

    if (query14) {

        pdbClient.registerType ("libraries/libQ14Agg.so", errMsg);
        pdbClient.registerType ("libraries/libQ14AggOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ14Join.so", errMsg);
        pdbClient.registerType ("libraries/libQ14JoinOut.so", errMsg);
        pdbClient.registerType ("libraries/libQ14ValueClass.so", errMsg);
        pdbClient.registerType ("libraries/libQ14LineItemSelection.so", errMsg);

    }

    bool query17 = true;

    if (query17) {

        pdbClient.registerType ("libraries/libQ17JoinedPartLineItem.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PartLineItemAvgJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PartSelection.so", errMsg);
        pdbClient.registerType ("libraries/libQ17LineItemAvgQuantity.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PartLineItemJoin.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PriceSum.so", errMsg);
        pdbClient.registerType ("libraries/libQ17PartLineItemIdentitySelection.so", errMsg);
    }

    bool query22 = true;

    if (query22) {

         pdbClient.registerType("libraries/libQ22AggregatedCntryBal.so", errMsg);
         pdbClient.registerType("libraries/libQ22CustomerAccbalAvg.so", errMsg);
         pdbClient.registerType("libraries/libQ22OrderCountSelection.so", errMsg);
         pdbClient.registerType("libraries/libQ22CntryBalAgg.so", errMsg);
         pdbClient.registerType("libraries/libQ22JoinedCntryBal.so", errMsg);
         pdbClient.registerType("libraries/libQ22CntryBalJoin.so", errMsg);
         pdbClient.registerType("libraries/libQ22OrderCountPerCustomer.so", errMsg);

    }


}

void removeSets (PDBClient & pdbClient) {

    std::string errMsg;
    pdbClient.removeSet("tpch", "customer", errMsg);
    pdbClient.removeSet("tpch", "lineitem", errMsg);
    pdbClient.removeSet("tpch", "nation", errMsg);
    pdbClient.removeSet("tpch", "order", errMsg);
    pdbClient.removeSet("tpch", "part", errMsg);
    pdbClient.removeSet("tpch", "partsupp", errMsg);
    pdbClient.removeSet("tpch", "region", errMsg);
    pdbClient.removeSet("tpch", "supplier", errMsg);

}


int main(int argc, char* argv[]) {

    bool whetherToRegisterLibraries = true;
    if (argc > 1) {
        if (strcmp(argv[1], "N") == 0) {
            whetherToRegisterLibraries = false;
        }
    }

    bool whetherToCreateSets = true;
    if (argc > 2) {
        if (strcmp(argv[2], "N") == 0) {
           whetherToCreateSets = false;
        }
    }

    bool whetherToRemoveData = false;
    if (argc > 3) {
        if (strcmp(argv[3], "Y") == 0) {
           whetherToRemoveData = true;
        }
    }

    if ((argc > 4) || (argc == 1)) {
       std::cout << "Usage: #whetherToRegisterLibraries (Y/N)" 
                 << " #whetherToCreateSets (Y/N)"
                 << " #whetherToRemoveData (Y/N)" << std::endl;
    }

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

    if (whetherToRegisterLibraries == true) {
        registerLibraries (pdbClient);
    }

    if (whetherToCreateSets == true) {
        createSets (pdbClient, whetherToStartTraining);
    }

    if (whetherToRemoveData == true) {
        removeSets(pdbClient);
    }    

    // Clean up the SO files.
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std::cout << "Can't cleanup so files" << std::endl;
    }
}

#endif
