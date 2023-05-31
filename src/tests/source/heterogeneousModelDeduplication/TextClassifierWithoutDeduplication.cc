
#ifndef TEXT_CLASSIFICATION_WITHOUT_DEDUPLICATION_CC
#define TEXT_CLASSIFICATION_WITHOUT_DEDUPLICATION_CC

#include "PDBString.h"
#include "PDBMap.h"
#include "DataTypes.h"
#include "TensorBlockIndex.h"
#include "InterfaceFunctions.h"
#include "PDBClient.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"
#include "FFMatrixBlockScanner.h"
#include "FFMatrixWriter.h"
#include "FFAggMatrix.h"
#include "FFAggMatrixToOneMatrix.h"
#include "FFTransposeMult.h"
#include "SemanticClassifierSingleBlock.h"
#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

using namespace pdb;


void createData (pdb::PDBClient pdbClient, std::string databaseName, int block_x, int block_y, int vocab_size, int embedding_dimension, int batchSize) {

	std::string errMsg;

        ff::createDatabase(pdbClient, databaseName);

        //create input set
        pdbClient.createSet<FFMatrixBlock>(databaseName, "inputs", errMsg,
                     DEFAULT_PAGE_SIZE, "inputs", nullptr, nullptr, false);

        // create weights set
        pdbClient.removeSet(databaseName, "weights", errMsg);
        pdbClient.createSet<FFMatrixBlock>(databaseName, "weights",
                                           errMsg, DEFAULT_PAGE_SIZE, "weights",
                                           nullptr, nullptr, false);
        // batch_size x vocab_size
        std::cout << "To load matrix for " << databaseName << "::inputs"
                  << std::endl;
        ff::loadMatrix(pdbClient, databaseName, "inputs", batchSize,
                       vocab_size, block_x, block_y, false, false, errMsg);

        // embedding_dimension x vocab_size
        std::cout << "To load matrix for " << databaseName << "::weights"
                  << std::endl;
        ff::loadMatrix(pdbClient, databaseName, "weights",
                       embedding_dimension, vocab_size, block_x, block_y, false,
                       false, errMsg);

}

void runWorkload (pdb::PDBClient pdbClient, std::string databaseName, int embedding_dimension) {

    std::string errMsg;

    // create output set
    pdbClient.removeSet(databaseName, "outputs", errMsg);
    pdbClient.createSet<FFMatrixBlock>(databaseName, "outputs", errMsg,
                                       DEFAULT_PAGE_SIZE, "outputs", nullptr,
                                       nullptr, false);

    pdbClient.removeSet(databaseName, "intermediate", errMsg);
    pdbClient.createSet<FFMatrixBlock>(databaseName, "intermediate", errMsg,
                                       DEFAULT_PAGE_SIZE, "intermediate", nullptr,
                                       nullptr, false);

    auto begin = std::chrono::high_resolution_clock::now();

    // make the reader
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>(databaseName, "weights");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>(databaseName, "inputs");

    // make the transpose multiply join
    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the transpose multiply aggregation
    pdb::Handle<pdb::Computation> myAggregation =
        pdb::makeObject<FFAggMatrix>();

    myAggregation->setOutput(databaseName, "intermediate");
    myAggregation->setInput(join);

    pdb::Handle<pdb::Computation> readC =
        makeObject<FFMatrixBlockScanner>(databaseName, "intermediate");

    // merge the all FFMatrixcBlocks to one single FFMatrix
    pdb::Handle<pdb::Computation> myAggregation1 =
        pdb::makeObject<FFAggMatrixToOneMatrix>();
    myAggregation1->setInput(readC);

    // make the classifier
    uint32_t sizeDense0 = 16;
    uint32_t sizeDense1 = 1;



    // SemanticClassifierSingleBlock takes the input as FFSingleMatrix
    pdb::Handle<pdb::Computation> classifier =
        pdb::makeObject<SemanticClassifierSingleBlock>(embedding_dimension, sizeDense0, sizeDense1);
    classifier->setInput(myAggregation1);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = nullptr;
    myWriter =
        pdb::makeObject<FFMatrixWriter>(databaseName, "outputs");
    myWriter->setInput(classifier);

    bool materializeHash = false;

    auto exe_begin = std::chrono::high_resolution_clock::now();

    // run the computation
    if (!pdbClient.executeComputations(errMsg, databaseName, materializeHash,
                                   myAggregation)) {
        cout << "Computation failed. Message was: " << errMsg << "\n";
        exit(1);
    }

        // run the computation
    if (!pdbClient.executeComputations(errMsg, "aggregation-1", materializeHash,
                                   myWriter)) {
        cout << "Computation failed. Message was: " << errMsg << "\n";
        exit(1);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "****Text Classification End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end -
                                                                          begin)
                     .count()
              << " secs." << std::endl;

    std::cout << "****Text Classification Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(
                     end - exe_begin)
                     .count()
              << " secs." << std::endl;

    // verify the results
    ff::print_stats(pdbClient, databaseName, "outputs");
    ff::print(pdbClient, databaseName, "outputs");

}


int main(int argc, char *argv[]) {


    bool loadData = true;
    if (argc > 1) {
        if (strcmp(argv[1], "N") == 0) {
            loadData = false;
        }
    }

    int workloadToRun = 0;
    if (argc > 2) {
	workloadToRun = atoi(argv[2]);
    }

    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger =
        make_shared<pdb::PDBLogger>("workloadLog");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

    int block_x = 50;
    int block_y = 10000;
    int batch_size = 100;

    pdb::makeObjectAllocatorBlock(128*1024*1024, true);

    if (loadData) {

        ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
        ff::loadLibrary(pdbClient, "libraries/libFFSingleMatrix.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
        ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
        ff::loadLibrary(pdbClient, "libraries/libFFAggMatrixToOneMatrix.so");
        ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
        ff::loadLibrary(pdbClient, "libraries/libSemanticClassifierSingleBlock.so");

	createData(pdbClient, "nnlm-128", block_x, block_y, 963812, 128, batch_size);
	createData(pdbClient, "nnlm-50", block_x, block_y, 963812, 50, batch_size);
        createData(pdbClient, "wiki-250", block_x, block_y, 1009375, 250, batch_size);
        createData(pdbClient, "wiki-500", block_x, block_y, 1009375, 500, batch_size);

    }

    switch(workloadToRun) {
    
	    case 0: 
		    break;
	    case 1:
		    runWorkload(pdbClient, "nnlm-128", 128);
		    break;
	    case 2:
		    runWorkload(pdbClient, "nnlm-50", 50);
		    break;
	    case 3:
		    runWorkload(pdbClient, "wiki-250", 250);
		    break;
	    case 4:
		    runWorkload(pdbClient, "wiki-500", 500);
		    break;
            default:
                    std::cout << "No workload to run" << std::endl;		    
    
    }

}


#endif
