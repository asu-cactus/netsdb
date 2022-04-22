#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "PDBClient.h"

#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"

#ifdef WINDOWS
#include <direct.h>
	#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

using namespace std;

std::string get_current_dir() {
    char buff[FILENAME_MAX]; //create string buffer to hold path

    GetCurrentDir( buff, FILENAME_MAX );
    string current_working_dir(buff);
    return current_working_dir;
}

int main(int argc, char *argv[]) {

    bool reloadData = true; // If set to false, we can directly run the inference with pre-loaded library and data
    string errMsg;
    string input_path, labels_path, w_path, b_path;
    int block_x, block_y, batch_size;
    int numFeatures, numNeurons, numLabels;
    if(argc<4) {
        cout << "Usage: reload(N/Y) blockDimensionX blockDimensionY batchSize numFeatures numNeurons numLabels"
                "path/to/weights/and/bias(leave empty if generate random)"
             << endl;
        exit(-1);
    }

    cout << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << " " << argv[5] << " " << argv[6] << " " << argv[7] << " " << argv[8] << endl;

    if (strcmp(argv[1], "N")==0)
        reloadData = false;
    block_x = atoi(argv[2]);
    block_y = atoi(argv[3]);
    batch_size = atoi(argv[4]);
    numFeatures = atoi(argv[5]);
    numNeurons = atoi(argv[6]); // For LogReg, numFeatures and numNeurons can be made same
    numLabels = atoi(argv[7]);

    cout << "Using Block Dimensions " << block_x << ", " << block_y << endl;

    bool generate = false; // I want to reload existing weights, biases from TF

    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("FFClientLog");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);


    //given the small input size, I tuned all page size to be 1 megabyte

    if(reloadData) {
        ff::createDatabase(pdbClient, "ff");

	ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
	ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
	ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
	ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
	ff::loadLibrary(pdbClient, "libraries/libFFInputLayerJoin.so");
	ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
	ff::loadLibrary(pdbClient, "libraries/libFFTransposeBiasSumSigmoid.so");


        ff::createSet(pdbClient, "ff", "inputs", "inputs", 1);
        ff::createSet(pdbClient, "ff", "label", "label", 1);

        ff::createSet(pdbClient, "ff", "w", "W", 1);
        ff::createSet(pdbClient, "ff", "b", "B", 1);
    }

    ff::createSet(pdbClient, "ff", "output", "Output", 1);


    std::cout << "Generate & ReloadData: " << generate << " " << reloadData << std::endl;

    if(reloadData) { // First time, we reload data from .out files
        std::cout << "Loading data from folder: " << string(argv[8]) << std::endl;
        input_path = string(argv[8]) + "/input.out"; // input.out is col-major
        w_path = string(argv[8]) + "/weight.out";
        b_path = string(argv[8]) + "/bias.out"; // same length as inputs & labels
        std::cout << input_path << " " << labels_path << " " << w_path << " " << b_path << std::endl;

	//5000x6
        (void)ff::load_matrix_data(pdbClient, input_path, "ff", "inputs", block_x, numFeatures, true, true, errMsg);
        //6x1
	(void)ff::load_matrix_data(pdbClient, w_path, "ff", "w", numFeatures, block_y, true, true, errMsg);
        //5000x1
	(void)ff::load_matrix_data(pdbClient, b_path, "ff", "b", block_x, 1, true, true, errMsg);

	ff::print(pdbClient, "ff", "inputs");
	ff::print(pdbClient, "ff", "w");
	ff::print(pdbClient, "ff", "b");


    } 


    auto begin = std::chrono::high_resolution_clock::now();
    ff::inference_unit_log_reg1(pdbClient, "ff", "w", "inputs", "b", "output"); // Need to write my own for LogReg now
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "*****FFTest End-to-End Time Duration: ****" << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count() << " secs." << std::endl;

    ff::print(pdbClient, "ff", "output"); 
    
    vector<vector<double>> labels_test;

    labels_path = string(argv[8]) + "/label.out";
    std::cout << "Checkpoint: If loop for load_matrix_from_file" << std::endl;
    ff::load_matrix_from_file(labels_path, labels_test);

    int total_count = 0;
    int correct_count = 0;
    {
        pdb::UseTemporaryAllocationBlock tempBlock{1024*1024*128};

        auto iterator = pdbClient.getSetIterator<FFMatrixBlock>("ff", "output");

        for(auto r:iterator) {  // How to iterate over this and labels_test?
            total_count++;
            double *data = r->getRawDataHandle()->c_ptr();
            for (int i = 0; i < block_x; i++) {
	          int curData = data[i];
	          int label = labels_test[i][0];
	          std::cout << curData << ":" << label << std::endl;
	          if (curData == label) {
		    correct_count++;
                  }
            }
            cout << "Accuracy: " << correct_count << "/" << block_x << std::endl;
            std::cout << "count=" << total_count << std::endl;
       }
    }

    return 0;
}
