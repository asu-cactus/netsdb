
#ifndef TEST_WIKI500_YELP_CC
#define TEST_WIKI500_YELP_CC

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


int main(int argc, char *argv[]) {
    bool loadData = true;
    if (argc > 1) {
        if (strcmp(argv[1], "N") == 0) {
            loadData = false;
        }
    }
    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger =
        make_shared<pdb::PDBLogger>("WIKI500_YELP_Log");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);
    std::string errMsg;

    int block_x = 50;
    int block_y = 10000;
    int vocab_size = 1009375;
    int embedding_dimension = 500;

    int batchSize = 100;

    pdb::makeObjectAllocatorBlock(128*1024*1024, true);

    if (loadData) {
        ff::createDatabase(pdbClient, "wiki-500");
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

        //create input set
        pdbClient.createSet<FFMatrixBlock>("wiki-500", "inputs", errMsg,
                     DEFAULT_PAGE_SIZE, "inputs", nullptr, nullptr, false);

        // create weights set
        pdbClient.removeSet("wiki-500", "weights", errMsg);
        pdbClient.createSet<FFMatrixBlock>("wiki-500", "weights",
                                           errMsg, DEFAULT_PAGE_SIZE, "weights",
                                           nullptr, nullptr, false);
        // batch_size x vocab_size
        std::cout << "To load matrix for wiki-500::inputs"
                  << std::endl;
        ff::loadMatrix(pdbClient, "wiki-500", "inputs", batchSize,
                       vocab_size, block_x, block_y, false, false, errMsg);

        // embedding_dimension x vocab_size
        std::cout << "To load matrix for wiki-500::weights"
                  << std::endl;
        ff::loadMatrix(pdbClient, "wiki-500", "weights",
                       embedding_dimension, vocab_size, block_x, block_y, false,
                       false, errMsg);
    }

    auto begin = std::chrono::high_resolution_clock::now();
    // create output set
    pdbClient.removeSet("wiki-500", "outputs", errMsg);
    pdbClient.createSet<FFMatrixBlock>("wiki-500", "outputs", errMsg,
                                       DEFAULT_PAGE_SIZE, "outputs", nullptr,
                                       nullptr, false);
    // make the reader
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("wiki-500", "weights");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("wiki-500", "inputs");

    // make the transpose multiply join
    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the transpose multiply aggregation
    pdb::Handle<pdb::Computation> myAggregation =
        pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    // merge the all FFMatrixcBlocks to one single FFMatrix
    pdb::Handle<pdb::Computation> myAggregation1 =
        pdb::makeObject<FFAggMatrixToOneMatrix>();
    myAggregation1->setInput(myAggregation);

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
        pdb::makeObject<FFMatrixWriter>("wiki-500", "outputs");
    myWriter->setInput(classifier);

    bool materializeHash = false;

    auto exe_begin = std::chrono::high_resolution_clock::now();

    // run the computation
    if (!pdbClient.executeComputations(errMsg, "WIKI500_YELP", materializeHash,
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
    ff::print_stats(pdbClient, "wiki-500", "outputs");
    ff::print(pdbClient, "wiki-500", "outputs");
}


#endif
