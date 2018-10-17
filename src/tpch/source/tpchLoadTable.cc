#ifndef TPCH_LOAD_TABLE_CC
#define TPCH_LOAD_TABLE_CC

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


void createSet (PDBClient & pdbClient, std::string dataType) {

    std::string errMsg;
    pdbClient.createDatabase("tpch", errMsg);

    if (dataType == "Customer") {
         pdbClient.removeSet("tpch", "customer", errMsg);
         std::cout << "to create set for Customer" << std::endl;
         bool customerRes = pdbClient.createSet<Customer>("tpch", "customer", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadCustomer");
    } else if (dataType == "LineItem") {
         pdbClient.removeSet("tpch", "lineitem", errMsg);
         std::cout << "to create set for LineItem" << std::endl;
         bool lineitemRes = pdbClient.createSet<LineItem>("tpch", "lineitem", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadLineItem");
    } else if (dataType == "Nation") {
         pdbClient.removeSet("tpch", "nation", errMsg);
         std::cout << "to create set for Nation" << std::endl;
         bool nationRes = pdbClient.createSet<Nation>("tpch", "nation", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadNation");
    } else if (dataType == "Order") {
         pdbClient.removeSet("tpch", "order", errMsg);
         std::cout << "to create set for Order" << std::endl;
         bool orderRes = pdbClient.createSet<Order>("tpch", "order", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadOrder");
    } else if (dataType == "Part") {
         pdbClient.removeSet("tpch", "part", errMsg);
         std::cout << "to create set for Part" << std::endl;
         bool partRes = pdbClient.createSet<Part>("tpch", "part", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadPart");
    } else if (dataType == "PartSupp") {
         pdbClient.removeSet("tpch", "partsupp", errMsg);
         std::cout << "to create set for PartSupp" << std::endl;
         bool partsuppRes = pdbClient.createSet<PartSupp>("tpch", "partsupp", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadPartSupp");
    } else if (dataType == "Region") {
         pdbClient.removeSet("tpch", "region", errMsg);
         std::cout << "to create set for Region" << std::endl;
         bool regionRes = pdbClient.createSet<Region>("tpch", "region", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadRegion");
    } else if (dataType == "Supplier") {
         pdbClient.removeSet("tpch", "supplier", errMsg);
         std::cout << "to create set for Supplier" << std::endl;
         bool supplierRes = pdbClient.createSet<Supplier>("tpch", "supplier", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadSupplier");
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

    //table to load
    std::string tableToLoad = "";
    if (argc > 2) {
        tableToLoad = std::string(argv[2]);
    }

    //lambda for partitioning the table to load
    std::string jobName = "";
    if (argc > 3) {
        jobName = std::string(argv[3]);
    }

    std::string compName = "";
    if (argc > 4) {
        compName = std::string(argv[4]);
    }

    std::string lambdaName = "";
    if (argc > 5) {
        lambdaName = std::string(argv[5]);
    }

    bool whetherToCreateSet = true;
    if (argc > 6) {
        if (strcmp(argv[6], "N") == 0) {
           whetherToCreateSet = false;
        }
    }

    bool whetherToAddData = true;
    if (argc > 7) {
        if (strcmp(argv[7], "N") == 0) {
           whetherToAddData = false;
        }
    }

    bool whetherToRemoveData = false;
    if (argc > 8) {
        if (strcmp(argv[8], "Y") == 0) {
           whetherToRemoveData = true;
        }
    }


    if ((argc > 9) || (argc == 1)) {
       std::cout << "Usage: #tpchDirectory #tableToLoad #jobName #computationName #lambdaName"
                 << " #whetherToCreateSet (Y/N) #whetherToAddData (Y/N)"
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


    if (whetherToCreateSet == true) {
        createSet (pdbClient, tableToLoad);
    }

    if (whetherToAddData == true) {
        if (tableToLoad == "Customer") {
            loadData(pdbClient, tpchDirectory + "/customer.tbl", "Customer");
        } else if (tableToLoad == "LineItem") {
            loadData(pdbClient, tpchDirectory + "/lineitem.tbl", "LineItem");
        } else if (tableToLoad == "Nation") {
            loadData(pdbClient, tpchDirectory + "/nation.tbl", "Nation");
        } else if (tableToLoad == "Order") {
            loadData(pdbClient, tpchDirectory + "/orders.tbl", "Order");
        } else if (tableToLoad == "Part") {
            loadData(pdbClient, tpchDirectory + "/part.tbl", "Part");
        } else if (tableToLoad == "PartSupp") {
            loadData(pdbClient, tpchDirectory + "/partsupp.tbl", "PartSupp");
        } else if (tableToLoad == "Region") {
            loadData(pdbClient, tpchDirectory + "/region.tbl", "Region");
        } else if (tableToLoad == "Supplier") {
            loadData(pdbClient, tpchDirectory + "/supplier.tbl", "Supplier");
        }
        std::cout << "to flush data to disk" << std::endl;
        std::string errMsg;
        pdbClient.flushData(errMsg);

    }

    if (whetherToRemoveData == true) {
        removeSet(pdbClient, tableToLoad);
    }    


    // Clean up the SO files.
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std::cout << "Can't cleanup so files" << std::endl;
    }
}

#endif
