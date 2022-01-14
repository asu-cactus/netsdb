#ifndef TEST_MATERIALIZED_MODEL_CC
#define TEST_MATERIALIZED_MODEL_CC

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "PDBClient.h"
#include "StorageClient.h"
#include "TreeNode.h"
#include "PDBClient.h"
#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SpecializedBC.h"
#include "GenericDT.h"
#include "SimpleFF.h"

using namespace pdb;

int main(int argc, char *argv[]) {

	int rowNum = 569;
	int colNum = 31;
	int block_x = 569;
	int block_y = 31;

	string errMsg;

	makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

	string masterIp = "localhost";
	pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
	pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
	pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

	pdbClient.registerType("libraries/libTreeNode.so", errMsg);

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
    pdbClient.flushData(errMsg);

    bool results = pdbClient.MM_getSet(dbName, setName, errMsg);
    std::cout << "Here is the MM_getSet results: " << results << std::endl;

    ff::createDatabase(pdbClient, "decisiontreeBC");

    ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixPartitioner.so");

    ff::createSet(pdbClient, "decisiontreeBC", "inputs", "inputs", 64);
    ff::createSet(pdbClient, "decisiontreeBC", "labels", "labels", 64);

    std::cout << "To load shared libraries of rules" << std::endl;
    ff::loadLibrary(pdbClient, "libraries/libGenericDT.so");

    auto begin = std::chrono::high_resolution_clock::now();

    std::cout << "To load matrix for decision tree inputs" << std::endl;
    ff::loadMatrix(pdbClient, "decisiontreeBC", "inputs", rowNum, colNum, block_x,
                   block_y, false, false, errMsg);

    std::cout << "To print the inputs" << std::endl;
    ff::print(pdbClient, "decisiontreeBC", "inputs");

    pdb::Handle<pdb::Computation> inputMatrix = pdb::makeObject<FFMatrixBlockScanner>("decisiontreeBC", "inputs");

    std::cout << "To make object of decision tree shared libraries" << std::endl;
    string fileName = "trees/"+dbName+setName+".csv";
    pdb::Handle<pdb::Computation> genericDT = pdb::makeObject<GenericDT>(fileName);
    genericDT->setInput(inputMatrix);

    std::cout << "To set the Computation" << std::endl;
    pdb::Handle<pdb::Computation> labelWriter = nullptr;
    labelWriter = pdb::makeObject<FFMatrixWriter>("decisiontreeBC", "labels");
    labelWriter->setInput(genericDT);

    bool materializeHash = false;
    std::cout << "To run the Computation" << std::endl;
    auto exe_begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.executeComputations(errMsg, "decisiontreeBC", materializeHash, labelWriter)) {
    	cout << "Computation failed. Message was: " << errMsg << "\n";
    	exit(1);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "****Decision Tree End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

    std::cout << "****Decision Tree Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - exe_begin).count()
              << " secs." << std::endl;

    //verify the results
    std::cout << "To print the status" << std::endl;
    ff::print_stats(pdbClient, "decisiontreeBC", "labels");
    std::cout << "To print the results" << std::endl;
    ff::print(pdbClient, "decisiontreeBC", "labels");
    return 0;
}

#endif