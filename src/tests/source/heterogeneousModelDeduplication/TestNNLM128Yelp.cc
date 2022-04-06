
#ifndef TEST_NNLM128_YELP_CC
#define TEST_NNLM128_YELP_CC

#include "DataTypes.h"
#include "FFAggMatrix.h"
#include "FFAggMatrixByCol.h"
#include "FFMatrixBlock.h"
#include "FFMatrixBlockScanner.h"
#include "FFMatrixUtil.h"
#include "FFMatrixWriter.h"
#include "FFTransposeMult.h"
#include "InterfaceFunctions.h"
#include "PDBClient.h"
#include "PDBMap.h"
#include "PDBString.h"
#include "SemanticClassifierSingleBlock.h"
#include "SimpleFF.h"
#include "TensorBlockIndex.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

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
        make_shared<pdb::PDBLogger>("NNLM128_YELP_Log");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);
    std::string errMsg;

    int block_x = 50;
    int block_y = 10000;
    int vocab_size = 963812;
    int embedding_dimension = 128;

    int batch_size = 100;

    if (loadData) {
        ff::createDatabase(pdbClient, "text-classification");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
        ff::loadLibrary(pdbClient, "libraries/libFFSingleMatrix.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
        ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
        ff::loadLibrary(pdbClient, "libraries/libFFAggMatrixByCol.so");
        ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
        ff::loadLibrary(pdbClient, "libraries/libSemanticClassifierSingleBlock.so");

        //create input set
        pdbClient.createSet<FFMatrixBlock>("text-classification", "inputs", errMsg,
                     DEFAULT_PAGE_SIZE, "inputs", nullptr, nullptr, false);

        // create weights set 
        pdbClient.removeSet("text-classification", "weights", errMsg);
        pdbClient.createSet<FFMatrixBlock>("text-classification", "weights",
                                           errMsg, DEFAULT_PAGE_SIZE, "weights",
                                           nullptr, nullptr, false);

        // batch_size x vocab_size
        std::cout << "To load matrix for text-classification::inputs"
                  << std::endl;
        ff::loadMatrix(pdbClient, "text-classification", "inputs", batch_size,
                       vocab_size, block_x, block_y, false, false, errMsg);

        // embedding_dimension x vocab_size
        std::cout << "To load matrix for text-classification::weights"
                  << std::endl;
        ff::loadMatrix(pdbClient, "text-classification", "weights",
                       embedding_dimension, vocab_size, block_x, block_y, false,
                       false, errMsg);
    }

    auto begin = std::chrono::high_resolution_clock::now();
    // create output set
    pdbClient.removeSet("text-classification", "outputs", errMsg);
    pdbClient.createSet<FFMatrixBlock>("text-classification", "outputs", errMsg,
                                       DEFAULT_PAGE_SIZE, "outputs", nullptr,
                                       nullptr, false);

    // make the reader
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("text-classification", "weights");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("text-classification", "inputs");

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
        pdb::makeObject<FFAggMatrixByCol>();
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
        pdb::makeObject<FFMatrixWriter>("text-classification", "outputs");
    myWriter->setInput(classifier);


    auto exe_begin = std::chrono::high_resolution_clock::now();
    bool materializeHash = false;

    // run the computation
    if (!pdbClient.executeComputations(errMsg, "NNLM128_IMDB", materializeHash,
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
    ff::print_stats(pdbClient, "text-classification", "outputs");
    ff::print(pdbClient, "text-classification", "outputs");
}
#endif
