#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "PDBClient.h"
#include "SimpleFF.h"
#include "FFTransposeMult.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "HybridFullyConnectedNetwork.h"
#include "WriteUserSet.h"
#include "FFAggMatrix.h"

using namespace std;

int main(int argc, char *argv[]) {

  bool reloadData = true;
  string errMsg;
  string input_path, labels_path;
  int block_x, block_y, batch_size;

  if (argc < 3) {
    cout << "Usage: blockDimensionX blockDimensionY batchSize numFeatures numNeurons numLabels Y "
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


  if (argc >= 7) {
      if (strcmp(argv[7], "N")==0) {
          reloadData = false;
          std::cout << "WARNING: we do not reload data!" << std::endl;
      }
  }
  cout << "Using block dimensions " << block_x << ", " << block_y << endl;


  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("FFclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);



  if (reloadData) {

      ff::createDatabase(pdbClient, "ff");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
      ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
      ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
      pdbClient.registerType("libraries/libHybridFullyConnectedNetwork.so", errMsg);
      ff::createSet(pdbClient, "ff", "inputs", "inputs", 64);
      ff::createSet(pdbClient, "ff", "w1", "W1", 64);
  }

  ff::createSet(pdbClient, "ff", "output", "Output", 64);


  if (reloadData) {

    // X x features_size = None x 5000
    std::cout << "To load matrix for ff:inputs" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "inputs", batch_size, features, block_x,
                   block_y, false, false, errMsg);
    ff::loadMatrix(pdbClient, "ff", "w1", hid_size, features, block_x, block_y,
                   false, false, errMsg);
  }

  pdb::makeObjectAllocatorBlock((size_t)512 * (size_t)1024 * (size_t)1024, true);

  pdb::Handle<pdb::Computation> inputScanner =
      pdb::makeObject<pdb::ScanUserSet<FFMatrixBlock>>("ff", "inputs"); 

  pdb::Handle<pdb::Computation> weightScanner =
      pdb::makeObject<pdb::ScanUserSet<FFMatrixBlock>>("ff", "w1");

  pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
        join->setInput(0, inputScanner);
        join->setInput(1, weightScanner);

  pdb::Handle<pdb::Computation> myAggregation =
            pdb::makeObject<FFAggMatrix>();
        myAggregation->setInput(join);

  pdb::Handle<pdb::Computation> fullyConnectedNetwork = 
      pdb::makeObject<HybridFullyConnectedNetwork>(features, num_labels, hid_size, batch_size);

  fullyConnectedNetwork->setInput(0, myAggregation);

  pdb::Handle<pdb::Computation> writer =
      pdb::makeObject<pdb::WriteUserSet<FFMatrixBlock>>("ff", "output"); 

  //writer->setInput(0, fullyConnectedNetwork);
  writer->setInput(0, myAggregation);

  auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
  if (!pdbClient.executeComputations(errMsg, "fc-proj", true, 256, writer)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "*****FFTest End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;


  return 0;
}
