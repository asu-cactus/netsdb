#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "PDBClient.h"
#include "SimpleFF.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "FullyConnectedNetwork.h"
#include "WriteUserSet.h"

using namespace std;

int main(int argc, char *argv[]) {

  bool reloadData = true;
  string errMsg;
  string input_path, labels_path;
  int block_x, block_y, batch_size;

  if (argc < 3) {
    cout << "Usage: blockDimensionX blockDimensionY batchSize numFeatures numNeurons numLabels YY"
            "path/to/weights/and/bias(leave empty if generate random)"
         << endl;
    exit(-1);
  }

  block_x = atoi(argv[1]);
  block_y = atoi(argv[2]);

  batch_size = atoi(argv[3]);

  int features = atoi(argv[4]);
  int hid_size = atoi(argv[5]);
  int num_labels = atoi(argv[6]);


  if (argc >= 4) {
      if (strcmp(argv[3], "N")==0) {
          reloadData = false;
          std::cout << "WARNING: we do not reload data!" << std::endl;
      }
  }
  cout << "Using block dimensions " << block_x << ", " << block_y << endl;

  bool generate = true;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("FFclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);



  if (reloadData) {

      ff::createDatabase(pdbClient, "ff");
      ff::setup(pdbClient, "ff");
      pdbClient.registerType("libraries/libFullyConnectedNetwork.so", errMsg);
      ff::createSet(pdbClient, "ff", "inputs", "inputs", 64);

  }

  ff::createSet(pdbClient, "ff", "output", "Output", 64);


  if (!generate && reloadData) {
    input_path = string(argv[4]) + "/input.out";
    labels_path = string(argv[4]) + "/labels.out";

    // load the input data
    ff::load_matrix_data(pdbClient, input_path, "ff", "inputs",
                                      block_x, block_y, false, false, errMsg);
  } else if (reloadData) {

    // X x features_size = None x 5000
    std::cout << "To load matrix for ff:inputs" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "inputs", batch_size, features, block_x,
                   block_y, false, false, errMsg);

  }


  pdb::Handle<pdb::Computation> inputScanner =
      pdb::makeObject<pdb::ScanUserSet<FFMatrixBlock>>("ff", "inputs"); 

  pdb::Handle<pdb::Computation> fullyConnectedNetwork = 
      pdb::makeObject<FullyConnectedNetwork>(features, num_labels, hid_size, batch_size);

  fullyConnectedNetwork->setInput(0, inputScanner);

  pdb::Handle<pdb::Computation> writer =
      pdb::makeObject<pdb::WriteUserSet<FFMatrixBlock>>("ff", "output"); 

  writer->setInput(0, fullyConnectedNetwork);

  auto begin = std::chrono::high_resolution_clock::now();

    // run the computation
  if (!pdbClient.executeComputations(errMsg, "fc-proj", writer)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "*****FFTest End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;



  vector<vector<double>> labels_test;

  if (!generate)
    ff::load_matrix_from_file(labels_path, labels_test);

  int correct = 0;
  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    auto it = pdbClient.getSetIterator<FFMatrixBlock>("ff", "output");

    for (auto r : it) {
      double *data = r->getRawDataHandle()->c_ptr();
      int i = 0;
      int j = r->getBlockRowIndex() * r->getRowNums();
      while (i < r->getRowNums() * r->getColNums()) {
        if (!generate && j >= labels_test.size())
          break;
        // double a = exp(data[i]);
        // double b = exp(data[i + 1]);
        // double sum = a + b;

        cout << data[i] << ", " << data[i + 1] << endl;

        if (!generate) {
          int pos1 = data[i] > data[i + 1] ? 0 : 1;
          int pos2 = labels_test[j][0] > labels_test[j][1] ? 0 : 1;

          if (pos1 == pos2)
            correct++;
        }

        i += r->getColNums();
        j++;
      }
    }

    if (!generate)
      cout << "Accuracy: " << correct << "/" << labels_test.size() << std::endl;
  }

  sleep(20);

  return 0;
}
