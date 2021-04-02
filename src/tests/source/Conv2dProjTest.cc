#include <algorithm>
#include <iostream>
#include <vector>

#include "Conv2DSelect.h"
#include "TensorData.h"
#include "PDBClient.h"
#include "PDBVector.h"
#include "ScanUserSet.h"
#include "WriteUserSet.h"

using namespace std;

void load_rnd_img(int x, int y, int z, int size, pdb::PDBClient &pdbClient,
                  pdb::String dbName, pdb::String setName) {
  std::string errMsg;
  pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

  pdb::Handle<pdb::Vector<unsigned int>> dimensions = pdb::makeObject<pdb::Vector<unsigned int>>();
  dimensions->push_back(z);
  dimensions->push_back(y);
  dimensions->push_back(x);


  pdb::Handle<pdb::Vector<pdb::Handle<pdb::TensorData>>> images = pdb::makeObject<pdb::Vector<pdb::Handle<pdb::TensorData>>>();  

  std::random_device rd;

  std::mt19937 e2(rd());

  std::uniform_real_distribution<> distp(0.0001, 0.5);
  std::uniform_real_distribution<> distn(-0.5, -0.0001);

  auto gen = std::bind(std::uniform_int_distribution<>(0, 1),
                       std::default_random_engine());


  for (int i = 0; i < size; i++) {
      try {
          pdb::Handle<pdb::TensorData> image = pdb::makeObject<pdb::TensorData>(3, dimensions);
          for (int c = 0; c < z; c++) {
             for (int h = 0; h < y; h++) {
                 for (int w = 0; w < x; w++) {
                      double data = (bool)gen() ? distn(e2) : distp(e2);
                      (*(image->rawData))[c * y * x + h * x + w] = data;
                 }
             }
          }
          images->push_back(image);
      } catch (pdb::NotEnoughSpace &e) {
          if (!pdbClient.sendData<pdb::TensorData>(
                pair<string, string>(setName, dbName), images, errMsg)) {
              cout << "Failed to send data to dispatcher server" << endl;
              exit(1);
          }
          i--;
          pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);
          dimensions = pdb::makeObject<pdb::Vector<unsigned int>>();
          dimensions->push_back(z);
          dimensions->push_back(y);
          dimensions->push_back(x);
          images = pdb::makeObject<pdb::Vector<pdb::Handle<pdb::TensorData>>>();
      }
  }

  if (!pdbClient.sendData<pdb::TensorData>(
       pair<string, string>(setName, dbName), images, errMsg)) {
       cout << "Failed to send data to dispatcher server" << endl;
       exit(1);
  }
  // to write back all buffered records
  pdbClient.flushData(errMsg);
}

int main(int argc, char *argv[]) {
  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger =
      make_shared<pdb::PDBLogger>("Conv2DclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  string errMsg;
  string dbName = "conv2d";
  string img_set = "img";

  // Load Libraries
  pdbClient.registerType("libraries/libConv2DSelect.so", errMsg);
  pdbClient.registerType("libraries/libTensorData.so", errMsg);

  //create input dataset
  pdbClient.removeDatabase(dbName, errMsg);
  if (!pdbClient.createDatabase(dbName, errMsg)) {
    cout << "Not able to create database " << dbName << ": " << errMsg << endl;
  } else {
    cout << "Created database " << dbName << endl;
  }

  pdbClient.removeSet(dbName, img_set, errMsg);
  if (!pdbClient.createSet<pdb::TensorData>(
            dbName, img_set, errMsg, (size_t)128 * (size_t)1024 * (size_t)1024)) {
      cout << "Not able to create set " + img_set + ": " + errMsg;
  } else {
      cout << "Created set " << img_set << ".\n";
  }


  //load data
  load_rnd_img(32, 32, 3, 5000, pdbClient, dbName, img_set);

 
  //create output dataset
  string feature_out_set = "feature_map";    
  pdbClient.removeSet(dbName, feature_out_set, errMsg);
  if (!pdbClient.createSet<pdb::TensorData>(
            dbName, feature_out_set, errMsg, (size_t)128 * (size_t)1024 * (size_t)1024)) {
      cout << "Not able to create set " + feature_out_set + ": " + errMsg;
  } else {
      cout << "Created set " << feature_out_set << ".\n";
  }


  //create scan computation
  pdb::Handle<pdb::Computation> imageScanner =
      pdb::makeObject<pdb::ScanUserSet<pdb::TensorData>>(dbName, img_set);

  pdb::Handle<pdb::Vector<unsigned int>> dimensions = pdb::makeObject<pdb::Vector<unsigned int>>(4);
  unsigned int x=3, y=3, z=3, n=1;
  dimensions->push_back(n);
  dimensions->push_back(z);
  dimensions->push_back(y);
  dimensions->push_back(x);
  pdb::Handle<TensorData> kernel = pdb::makeObject<TensorData>(4, dimensions);

  for (unsigned int i = 0; i < n*z*y*z; i++) {

     (*(kernel->rawData))[i] = 0.5;

  }

  //create select computation
  pdb::Handle<pdb::Computation> select = pdb::makeObject<Conv2DSelect>(kernel, "aten-conv2d");
  select->setInput(0, imageScanner);
  std::cout << "select's output type: "<< select->getOutputType() << std::endl;
  //create write computation
  pdb::Handle<pdb::Computation> myWriteSet =
      pdb::makeObject<pdb::WriteUserSet<pdb::TensorData>>(dbName, feature_out_set);
  myWriteSet->setInput(0, select);

  bool res = false;
  // run the computation
  if (!pdbClient.executeComputations(errMsg, "conv2d-proj", myWriteSet)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }  



  //test the output
  bool printResult = true;
  if (printResult == true) {
     std::cout << "to print result..." << std::endl;
     pdb::SetIterator<pdb::TensorData> result =
         pdbClient.getSetIterator<pdb::TensorData>(dbName, feature_out_set);
     std::cout << "Query results: ";
     int count = 0;
     for (auto a : result) {
         count++;
         std::cout << count << ":" << (*(a->dimensions))[0] << "," << (*(a->dimensions))[1] << "," << (*(a->dimensions))[2];
     }
     std::cout << "output count:" << count << "\n";
  }



  //remove the output set
  pdbClient.removeSet(dbName, feature_out_set, errMsg);  


  return 0;
}
