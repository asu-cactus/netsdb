#ifndef TEST_MATERIALIZED_MODEL_CC
#define TEST_MATERIALIZED_MODEL_CC

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "PDBClient.h"
#include "StorageClient.h"
#include "TreeNode.h"

using namespace pdb;

int main(int argc, char *argv[]) {

	string errMsg;

	makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

	string masterIp = "localhost";
	pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
	pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
	pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

	// create a new database
	string dbName = "materializedmodel_db";
    if (!pdbClient.createDatabase(dbName, errMsg)) {
        cout << "Not able to create database: " + errMsg;
        return -1;
    } else {
        cout << "Created materialized model database.\n";
    }

    // create a new set in the database
    string setName = "materializedmodel_set";
    if (!pdbClient.createSet<decisiontree::Node>(dbName, setName, errMsg)) {
        cout << "Not able to create set: " + errMsg;
        return -1;
    } else {
        cout << "Created materialized model set.\n";
    }

    pdb::Handle<pdb::Vector<pdb::Handle<decisiontree::Node>>> storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<decisiontree::Node>>>();
    try{
    	for(int i = 0; i < 5; i++){
    		pdb::Handle<decisiontree::Node> myData = pdb::makeObject<decisiontree::Node>(i,i,false,i,i,1.00);
    		storeMe->push_back(myData);
    	}
    } catch (pdb::NotEnoughSpace& e) {
    	if (!pdbClient.sendData<decisiontree::Node>(pair<string, string>(setName, dbName), storeMe, errMsg)) {
    		cout << "Not able to store data: " + errMsg;
    		return -1;
    	}
    }
    std::cout << "Stored the data!\n";

    pair<string, string> results = pdbClient.MM_getSet(dbName,setName);
    std::cout << results;
    pdbClient.flushData(errMsg);

    return 0;
}

#endif