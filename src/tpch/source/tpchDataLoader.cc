#ifndef TPCH_DATA_LOADER_CC
#define TPCH_DATA_LOADER_CC

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


#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

#define BLOCKSIZE (64 * MB)


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


void createSets (PDBClient & pdbClient, bool startTraining = false) {

    std::string errMsg;
    pdbClient.createDatabase("tpch", errMsg);
    pdbClient.removeSet("tpch", "customer", errMsg);
    std::cout << "to create set for Customer" << std::endl;
    bool customerRes = pdbClient.createSet<Customer>("tpch", "customer", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadCustomer");
    pdbClient.removeSet("tpch", "lineitem", errMsg);
    std::cout << "to create set for LineItem" << std::endl;
    bool lineitemRes = pdbClient.createSet<LineItem>("tpch", "lineitem", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadLineItem");
    pdbClient.removeSet("tpch", "nation", errMsg);
    std::cout << "to create set for Nation" << std::endl;
    bool nationRes = pdbClient.createSet<Nation>("tpch", "nation", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadNation");
    pdbClient.removeSet("tpch", "order", errMsg);
    std::cout << "to create set for Order" << std::endl;
    bool orderRes = pdbClient.createSet<Order>("tpch", "order", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadOrder");
    pdbClient.removeSet("tpch", "part", errMsg);
    std::cout << "to create set for Part" << std::endl;
    bool partRes = pdbClient.createSet<Part>("tpch", "part", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadPart");
    pdbClient.removeSet("tpch", "partsupp", errMsg);
    std::cout << "to create set for PartSupp" << std::endl;
    bool partsuppRes = pdbClient.createSet<PartSupp>("tpch", "partsupp", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadPartSupp");
    pdbClient.removeSet("tpch", "region", errMsg);
    std::cout << "to create set for Region" << std::endl;
    bool regionRes = pdbClient.createSet<Region>("tpch", "region", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadRegion");
    pdbClient.removeSet("tpch", "supplier", errMsg);
    std::cout << "to create set for Supplier" << std::endl;
    bool supplierRes = pdbClient.createSet<Supplier>("tpch", "supplier", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadSupplier");

    

    if ((startTraining == true) && ((customerRes || lineitemRes || nationRes || orderRes || partRes || partsuppRes || regionRes || supplierRes) == false)) {
        std::cout << "********************************" << std::endl;
        std::cout << errMsg << std ::endl;
        std::cout << "********************************" << std::endl;
        removeSets(pdbClient);
        exit(26);

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


int main(int argc, char* argv[]) {

    std::string tpchDirectory = "";
    if (argc > 1) {
        tpchDirectory = std::string(argv[1]);
    }

    bool whetherToRegisterLibraries = true;
    if (argc > 2) {
        if (strcmp(argv[2], "N") == 0) {
            whetherToRegisterLibraries = false;
        }
    }

    bool whetherToCreateSets = true;
    if (argc > 3) {
        if (strcmp(argv[3], "N") == 0) {
           whetherToCreateSets = false;
        }
    }

    bool whetherToAddData = true;
    if (argc > 4) {
        if (strcmp(argv[4], "N") == 0) {
           whetherToAddData = false;
        }
    }

    bool whetherToRemoveData = false;
    if (argc > 5) {
        if (strcmp(argv[5], "Y") == 0) {
           whetherToRemoveData = true;
        }
    }

    bool whetherToStartTraining = false;
    if (argc > 6) {
        if (strcmp(argv[6], "Y") == 0) {
           whetherToStartTraining = true;
        }
    }


    if ((argc > 7) || (argc == 1)) {
       std::cout << "Usage: #tpchDirectory #whetherToRegisterLibraries (Y/N)" 
                 << " #whetherToCreateSets (Y/N) #whetherToAddData (Y/N)"
                 << " #whetherToRemoveData (Y/N) #whetherToStartTraining (Y/N)" << std::endl;
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

    if (whetherToAddData == true) {
        loadData(pdbClient, tpchDirectory + "/customer.tbl", "Customer");
        loadData(pdbClient, tpchDirectory + "/lineitem.tbl", "LineItem");
        loadData(pdbClient, tpchDirectory + "/nation.tbl", "Nation");
        loadData(pdbClient, tpchDirectory + "/orders.tbl", "Order");
        loadData(pdbClient, tpchDirectory + "/part.tbl", "Part");
        loadData(pdbClient, tpchDirectory + "/partsupp.tbl", "PartSupp");
        loadData(pdbClient, tpchDirectory + "/region.tbl", "Region");
        loadData(pdbClient, tpchDirectory + "/supplier.tbl", "Supplier");

        std::cout << "to flush data to disk" << std::endl;
        std::string errMsg;
        pdbClient.flushData(errMsg);

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
