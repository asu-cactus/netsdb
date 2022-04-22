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
    bool reloadData = true; // To toggle if we need randomly generated data to test
    string errMsg;
    string input_path, labels_path, w_path, b_path;
    int block_x, block_y, batch_size;
    int numFeatures, numNeurons, numLabels;
    double sigmoidThreshold;
    if(argc<3) {
        cout << "Usage: blockDimensionX blockDimensionY batchSize numFeatures numNeurons numLabels"
                "path/to/weights/and/bias(leave empty if generate random)"
             << endl;
        exit(-1);
    }

//    cout << "\n" << get_current_dir() << endl;
//    cout << "Test ISTREAM" << endl;
//    ifstream is("TPCx_AI_Data/bias.out");
//    if(!is.is_open()) {
//        cout << "File not opened!!!" << endl;
//    }
//    string temp;
//    while(is.peek() != EOF) {
//        is >> temp;
//        cout << temp << endl;
//    }

    cout << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << " " << argv[5] << " " << argv[6] << " " << argv[7] << endl;
    block_x = atoi(argv[1]);
    block_y = atoi(argv[2]);
    batch_size = atoi(argv[3]);
    numFeatures = atoi(argv[4]);
    numNeurons = atoi(argv[5]); // For LogReg, numFeatures and numNeurons can be made same
    numLabels = atoi(argv[6]);
    sigmoidThreshold = 0.5; // atoi(argv[8]); // Currently, taking it to be 0.5

    cout << "Using Block Dimensions " << block_x << ", " << block_y << endl;

    bool generate = false; // I want to reload existing weights, biases from TF

    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("FFClientLog");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

    if(reloadData) {
        ff::createDatabase(pdbClient, "ff");
        ff::setup(pdbClient, "ff");

        ff::createSet(pdbClient, "ff", "inputs", "inputs", 512);
        ff::createSet(pdbClient, "ff", "label", "label", 512);

        ff::createSet(pdbClient, "ff", "w", "W", 64);
        ff::createSet(pdbClient, "ff", "b", "B", 64);
    }

    ff::createSet(pdbClient, "ff", "output", "Output", 512);
    ff::createSet(pdbClient, "ff", "y", "Y", 64);


    std::cout << "Generate & ReloadData: " << generate << " " << reloadData << std::endl;
    if(!generate && reloadData) { // First time, we reload data from .out files
        std::cout << "Loading data from folder: " << string(argv[7]) << std::endl;
        input_path = string(argv[7]) + "/input.out"; // input.out is col-major
        labels_path = string(argv[7]) + "/label.out";
        w_path = string(argv[7]) + "/weight.out";
        b_path = string(argv[7]) + "/bias.out"; // same length as inputs & labels
        std::cout << input_path << " " << labels_path << " " << w_path << " " << b_path << std::endl;

        (void)ff::load_matrix_data(pdbClient, input_path, "ff", "inputs", 5000, 6, true, true, errMsg);
        (void)ff::load_matrix_data(pdbClient, w_path, "ff", "w", 1, 6, true, true, errMsg);
        (void)ff::load_matrix_data(pdbClient, b_path, "ff", "b", 5000, 1, true, true, errMsg);
    } else if (reloadData) {
        std::cout << "To load matrix for ff:inputs" << std::endl;
        ff::loadMatrix(pdbClient, "ff", "inputs", batch_size, numFeatures, block_x, block_y, false, false, errMsg);
        std::cout << "To load matrix for ff:w" << std::endl;
        ff::loadMatrix(pdbClient, "ff", "w", numLabels, numNeurons, block_x, block_y, false, false, errMsg);
        std::cout << "To load matrix for ff:b" << std::endl;
        ff::loadMatrix(pdbClient, "ff", "b", numLabels, 1, block_x, 1, false, true, errMsg);
    }

    double dropout_rate = 0.5;

    auto begin = std::chrono::high_resolution_clock::now();
    ff::inference_unit_log_reg(pdbClient, "ff", "w", "inputs", "b", "output", dropout_rate); // Need to write my own for LogReg now
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "*****FFTest End-to-End Time Duration: ****" << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count() << " secs." << std::endl;

    vector<vector<double>> labels_test;

    if(!generate) {
        std::cout << "Checkpoint: If loop for load_matrix_from_file" << std::endl;
        ff::load_matrix_from_file(labels_path, labels_test);
    }

    int total_count = 0;
    int correct_count = 0;
    {
        pdb::UseTemporaryAllocationBlock tempBlock{1024*1024*128};

        auto iterator = pdbClient.getSetIterator<FFMatrixBlock>("ff", "output");

        for(auto r:iterator) {
            total_count = r->getColNums();
            if(total_count<100) {
                cout << "Label: " << labels_test[total_count][0] << " at Index: " << total_count << "  Data Row: ";
                double *data_temp = r->getRawDataHandle()->c_ptr();
                for(int j=0;j<r->getColNums();j++) {
                    cout << data_temp[j] << ", ";
                }
                cout << endl;
            }

            double *data = r->getRawDataHandle()->c_ptr();
            for(int i=0;i<r->getColNums();i++) {
                if((data[i]<sigmoidThreshold && labels_test[i][0]==0) || (data[i]>=sigmoidThreshold && labels_test[i][0]==1)) {
                    correct_count++;
                }
            }
            total_count++;
        }
        if (!generate)
            cout << "Accuracy: " << correct_count << "/" << total_count << std::endl;
        std::cout << "count=" << total_count << std::endl;
    }

    sleep(20);

    return 0;
}
