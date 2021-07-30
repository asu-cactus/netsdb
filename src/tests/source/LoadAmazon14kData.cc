//
// by Jia, Apr 2019
//

#include "PDBClient.h"
#include "RedditComment.h"

#include <string>
#include <random>
#include <vector>
#include <chrono>

using namespace pdb;

#include "FFMatrixUtil.h"

#include "FFMatrixData.h"
#include "FFMatrixMeta.h"
#include "FFMatrixBlock.h"

#include "AbstractIndexer.h"
#include "FFPageIndexer.h"

string getName(string name, int ver) { return name + "_m" + to_string(ver); }
string getName1(string name, int ver) { return name + "1_m" + to_string(ver); }

void clearSet(pdb::PDBClient &pdbClient, string dbname, int num_models) {
  string errMsg;

  for (int i = 0; i < num_models; i++) {
    string inputName = getName("inputs", i);
    string w1Name = getName("w1", i);
    string b1Name = getName("b1", i);
    string w2Name = getName("w2", i);
    string b2Name = getName("b2", i);

    pdbClient.removeSet(dbname, inputName, errMsg);
    pdbClient.removeSet(dbname, w1Name, errMsg);
    pdbClient.removeSet(dbname, b1Name, errMsg);
    pdbClient.removeSet(dbname, w2Name, errMsg);
    pdbClient.removeSet(dbname, b2Name, errMsg);
  }
}

void createSet(pdb::PDBClient &pdbClient, string dbname, int num_models, bool share, bool enablePartition) {
  string errMsg;
  int size = 128;

  for (int i = 0; i < num_models; i++) {
    string inputName = getName("inputs", i);
    string w1Name = getName("w1", i);
    string b1Name = getName("b1", i);
    string w2Name = getName("w2", i);
    string b2Name = getName("b2", i);
    string inputName1 = getName1("inputs", i);
    string w1Name1 = getName1("w1", i);
    string b1Name1 = getName1("b1", i);
    string w2Name1 = getName1("w2", i);
    string b2Name1 = getName1("b2", i);

    string compName1 = getName("inference-1", i);
    string compName2 = getName("inference-2", i);

    if (enablePartition) {
      pdb::makeObjectAllocatorBlock((size_t)128 * (size_t)1024 * (size_t)1024, true);
      Handle<LambdaIdentifier> inputLambda = pdb::makeObject<LambdaIdentifier>(compName1, "JoinComp_2", "methodCall_1");
      pdbClient.createSet<FFMatrixBlock>(dbname, inputName, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, inputName1, nullptr, inputLambda);

      Handle<LambdaIdentifier> w1Lambda = pdb::makeObject<LambdaIdentifier>(compName1, "JoinComp_2", "methodCall_0");

      if (share) {
        pdbClient.createSet<FFMatrixBlock>(dbname, w1Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, w1Name1, nullptr, w1Lambda, true);
      } else {
        pdbClient.createSet<FFMatrixBlock>(dbname, w1Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, w1Name1, nullptr, w1Lambda);
      }

      Handle<LambdaIdentifier> b1Lambda = pdb::makeObject<LambdaIdentifier>(compName1, "JoinComp_5", "methodCall_0");
      pdbClient.createSet<FFMatrixBlock>(dbname, b1Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, b1Name1, nullptr, b1Lambda);

      Handle<LambdaIdentifier> w2Lambda = pdb::makeObject<LambdaIdentifier>(compName1, "JoinComp_2", "methodCall_0");
      pdbClient.createSet<FFMatrixBlock>(dbname, w2Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, w2Name1, nullptr, w2Lambda);

      Handle<LambdaIdentifier> b2Lambda = pdb::makeObject<LambdaIdentifier>(compName1, "JoinComp_5", "methodCall_0");
      pdbClient.createSet<FFMatrixBlock>(dbname, b2Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, b2Name1, nullptr, b2Lambda);

    } else {

      pdbClient.createSet<FFMatrixBlock>(dbname, inputName, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, inputName1);
      if (share) {
        pdbClient.createSet<FFMatrixBlock>(dbname, w1Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, w1Name1, nullptr, nullptr, true);
      } else {
        pdbClient.createSet<FFMatrixBlock>(dbname, w1Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, w1Name1);
      }
      pdbClient.createSet<FFMatrixBlock>(dbname, b1Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, b1Name1);
      pdbClient.createSet<FFMatrixBlock>(dbname, w2Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, w2Name1);
      pdbClient.createSet<FFMatrixBlock>(dbname, b2Name, errMsg, (size_t)size * (size_t)1024 * (size_t)1024, b2Name1);

    }

  }
}

void loadSet(pdb::PDBClient &pdbClient, string dbname, int block_x, int block_y, int num_models,
                int batch_size, int label_size, int feature_size, int hd1size, bool share) {
  string errMsg;

  if (share) {
    vector<string> setNames;
    for (int i = 0; i < num_models; i++) {
      setNames.push_back(getName("w1", i));
    }

    // hidden_layer_1 size x features_size = <1k, 3k, 5k, 7k> x 597540
    ff::loadMatrix(pdbClient, dbname, setNames, hd1size, feature_size, block_x,
                  block_y, false, false, errMsg);
  }

  for (int i = 0; i < num_models; i++) {
    string inputName = getName("inputs", i);
    string w1Name = getName("w1", i);
    string b1Name = getName("b1", i);
    string w2Name = getName("w2", i);
    string b2Name = getName("b2", i);

    // batch_size x features_size = 1000 x 597540
    ff::loadMatrix(pdbClient, dbname, inputName, batch_size, feature_size,
                   block_x, block_y, false, false, errMsg, 64, true);
    if (!share) {
      // hidden_layer_1 size x features_size = <1k, 3k, 5k, 7k> x 597540
      ff::loadMatrix(pdbClient, dbname, w1Name, hd1size, feature_size, block_x,
                    block_y, false, false, errMsg, 64, true);
    }

    // labels_size x hidden_layer_1 = 14588 X <1k, 3k, 5k, 7k>
    ff::loadMatrix(pdbClient, dbname, w2Name, label_size, hd1size, block_x,
                   block_y, false, false, errMsg, 64, true);
    // hidden_layer_1 x 1
    ff::loadMatrix(pdbClient, dbname, b1Name, hd1size, 1, block_x, block_y,
                   false, true, errMsg, 64, false);
    // labels_size x 1
    ff::loadMatrix(pdbClient, dbname, b2Name, label_size, 1, block_x, block_y,
                   false, true, errMsg, 64, false);
  }
}

int main(int argc, char* argv[]) {
 
  // we put the error here
  string errMsg;

  // make sure we have the arguments
  if(argc < 12) {
    std::cout << "Usage : ./LoadAmazon14kData managerIP managerPort whetherToRegisterLibraries whetherToRemoveSet numModels blockDimensionX blockDimensionY share batchSize labelSize featureSize hiddenLayerSize enablePartition\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "whetherToRegisterLibraries - Y yes, N no\n";
    std::cout << "whetherToRemoveSet - Y yes, N no\n";
    std::cout << "numModels - number of FF models\n";
    std::cout << "blockDimensionX - block size x dimension\n";
    std::cout << "blockDimensionY - block size y dimension\n";
    std::cout << "share - enable sharing or not\n";
    std::cout << "batchSize - input data batch size\n";
    std::cout << "labelSize - number of labels\n";
    std::cout << "featureSize - number features\n";
    std::cout << "hiddenLayerSize - hidden layer size\n";

  }
  srand (time(NULL));
  //  get the manager address
  std::string managerIp = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  bool whetherToRegisterLibraries = true;
  if (strcmp(argv[3], "N")==0) {
      whetherToRegisterLibraries = false;
  }
  
  bool whetherToRemoveSet = true;
  if (strcmp(argv[4], "N")==0) {
      whetherToRemoveSet = false;
  }

  int numModels = std::atoi(argv[5]);
  int blockX = std::atoi(argv[6]);
  int blockY = std::atoi(argv[7]);
  
  cout << "Using block dimensions " << blockX << ", " << blockY << endl;

  bool enableSharing = false;
  if (strcmp(argv[8], "Y")==0) {
      enableSharing = true;
      std::cout << "Sharing first layer!" << std::endl;
  } else {
      std::cout << "Not Sharing first layer!" << std::endl;
  }

  int batchSize = std::atoi(argv[9]);
  int labelSize = std::atoi(argv[10]);
  int featureSize = std::atoi(argv[11]);
  int hiddenLayerSize = std::atoi(argv[12]);

  bool enablePartition = strcmp(argv[13], "Y") == 0; 

  cout << "Loading " << numModels << " Models with batchSize: " << batchSize
      << ", labelsSize: " << labelSize << ", featureSize: " << featureSize
      << ", hiddenLayerSize: " << hiddenLayerSize << endl;

  std::string dbName = "amazon14k";

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, managerIp, clientLogger);
  PDBClient pdbClient(port, managerIp, clientLogger, false, true);

  pdbClient.registerType("libraries/libFFMatrixMeta.so", errMsg);
  pdbClient.registerType("libraries/libFFMatrixData.so", errMsg);
  pdbClient.registerType("libraries/libFFMatrixBlock.so", errMsg);
  pdbClient.registerType("libraries/libFFPageIndexer.so", errMsg);

  // now, create a new database
  if (!catalogClient.databaseExists(dbName)) {
    pdbClient.createDatabase(dbName, errMsg);

    // Load the page indexer
    {
      const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

      pdb::Handle<pdb::AbstractIndexer> indexer = makeObject<FFPageIndexer>(10, 3);

      if (!pdbClient.addTypeIndexer<FFMatrixBlock>(dbName, indexer)) {
        cout << "Not able to create indexer: " + errMsg;
        exit(-1);
      } else {
        cout << "Created indexer.\n";
      }
    }
  }

  // now, create the output set
  if (whetherToRemoveSet == true) {
      clearSet(pdbClient, dbName, numModels);
  }

  createSet(pdbClient, dbName, numModels, enableSharing, enablePartition);

  loadSet(pdbClient, dbName, blockX, blockY, numModels, batchSize, labelSize, featureSize, hiddenLayerSize, enableSharing);

  pdbClient.printStorageStats(errMsg);
}
