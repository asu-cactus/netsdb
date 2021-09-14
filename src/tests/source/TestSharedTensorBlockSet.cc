
#ifndef TEST_SHARED_TENSOR_BLOCK_SET_CC
#define TEST_SHARED_TENSOR_BLOCK_SET_CC

#include "PDBString.h"
#include "PDBMap.h"
#include "TensorBlockIndex.h"
#include "InterfaceFunctions.h"
#include "PDBClient.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"

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

int main(int argc, char* argv[]) {

    bool loadData = true;
    if (argc > 1) {
        if (strcmp(argv[1], "N") == 0) {
           loadData = false;
	}
    }
    makeObjectAllocatorBlock(124 * 1024 * 1024, true);

    //create a shared set
     string masterIp = "localhost";
     pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("TestSharedSetLog");
     pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
     pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

     ff::createDatabase(pdbClient, "word2vec");
     ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
     ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
     ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
     std::string errMsg;
     pdbClient.createSet<FFMatrixBlock>("word2vec", "shared_weights", errMsg,
		     DEFAULT_PAGE_SIZE, "weights", nullptr, nullptr, true);

     int block_x = 1000;
     int block_y = 1000;
     int matrix1_totalNumBlock_x = 550;
     int matrix2_totalNumBlock_x = 550;
     int sharedNumBlock_x = 500;
     int numBlock_y = 1;

     
     if (loadData) {
     //load blocks to the shared set
     ff::loadMatrix(pdbClient, "word2vec", "shared_weights", sharedNumBlock_x*block_x, numBlock_y*block_y, block_x, block_y, false, false, errMsg);
      
     //create private set 1
     pdbClient.createSet<FFMatrixBlock>("word2vec", "weights1", errMsg,
                     DEFAULT_PAGE_SIZE, "weights1", nullptr, nullptr, false);
    
     //load blocks to the private set 1
     ff::loadMatrix(pdbClient, "word2vec", "weights1", (matrix1_totalNumBlock_x-sharedNumBlock_x)*block_x, numBlock_y*block_y, block_x, block_y, false, false, errMsg);

     sleep(30);

     //add the metadata of shared pages to the private set 1
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
		    "word2vec", "shared_weights", "FFMatrixBlock",
		   0, 0, 0, true, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   1, 0, 1, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   2, 0, 2, false, 0, errMsg);  
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   3, 0, 3, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   4, 0, 4, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   5, 0, 5, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   6, 0, 6, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   7, 0, 7, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   8, 0, 8, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   9, 0, 9, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   10, 0, 10, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   11, 0, 11, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   12, 0, 12, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   13, 0, 13, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   14, 0, 14, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   15, 0, 15, false, 0, errMsg);
/*

     //create private set 2
     pdbClient.createSet<FFMatrixBlock>("word2vec", "weights2", errMsg,
                     DEFAULT_PAGE_SIZE, "weights1", nullptr, nullptr, false);
    
     //load blocks to the private set 2
     ff::loadMatrix(pdbClient, "word2vec", "weights2", (matrix1_totalNumBlock_x-sharedNumBlock_x)*block_x, numBlock_y*block_y, block_x, block_y, false, false, errMsg);
    
     //add the metadata of shared pages to the private set 2
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   0, 0, 0, true, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   1, 0, 1, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   2, 0, 2, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   3, 0, 3, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   4, 0, 4, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   5, 0, 5, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   6, 0, 6, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   7, 0, 7, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   8, 0, 8, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   9, 0, 9, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   10, 0, 10, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   11, 0, 11, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   12, 0, 12, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   13, 0, 13, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   14, 0, 14, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   15, 0, 15, false, 0, errMsg);
     */
     }
     
     //scan private set 1
     const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

     auto it = pdbClient.getSetIterator<FFMatrixBlock>("word2vec", "weights1", true);
     int count = 0;
     for (auto r : it) {
         r->print();
	 count++;
     }
     std::cout << "count=" << count << std::endl;
     //scan private set 2
    
}

#endif
