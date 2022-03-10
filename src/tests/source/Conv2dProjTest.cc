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

void load_rnd_img(int x, int y, int z, int n, int size, pdb::PDBClient &pdbClient,
                  pdb::String dbName, pdb::String setName) {
  std::string errMsg;
  pdb::makeObjectAllocatorBlock(128 * 1024 * 1024, true);
 
  std::cout << "----------------inside load rnd img---------------: " << x << " " << y << " " << z << " " << n << " " << size << std::endl;
  pdb::Handle<pdb::Vector<unsigned int>> dimensions = pdb::makeObject<pdb::Vector<unsigned int>>();
  dimensions->push_back(n);
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
            pdb::Handle<pdb::TensorData> image = pdb::makeObject<pdb::TensorData>(4, dimensions);
            for (unsigned int i = 0; i < n*z*y*x; i++) {
                float data = (bool)gen() ? distn(e2) : distp(e2);
                (*(image->rawData))[i] = data;
            }
            images->push_back(image);
      } catch (pdb::NotEnoughSpace &e) {
          if (!pdbClient.sendData<pdb::TensorData>(
                pair<string, string>(setName, dbName), images, errMsg)) {
              cout << "Failed to send data to dispatcher server" << endl;
              exit(1);
          }
          i--;
          pdb::makeObjectAllocatorBlock(128 * 1024 * 1024, true);
          dimensions = pdb::makeObject<pdb::Vector<unsigned int>>();
          dimensions->push_back(n);
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

vector<string> splitString(string s) {
    vector<string> result;
    stringstream s_stream(s); //create string stream from the string
    while(s_stream.good()) {
      string substr;
      getline(s_stream, substr, ','); //get first string delimited by comma
      result.push_back(substr);
   }

   return result;
}

int main(int argc, char *argv[]) {

  bool addDataOrNot = true;

  bool createSetOrNot = true;

  int numImages = 100;

  string inputDimensions = "1,64,112,112";
  string kernelDimensions = "64,64,1,1";
  int n, c, w, h;
  int nk, ck, wk, hk;

  string mode = "aten-conv2d";
  //string mode = "eigen-spatial";

  if ( argc > 1 ) {

      if ( strcmp( argv[1], "Y" ) == 0 ) {

         addDataOrNot = true;

      } else {

         addDataOrNot = false;

      }
  }

  if ( argc > 2 ) {

      if ( strcmp( argv[2], "Y") == 0 ) {

         createSetOrNot = true;

      } else {

         createSetOrNot = false;

      }

  }

  if ( argc > 3 ) {

     numImages = atoi(argv[3]);

  }

  if ( argc > 4 ) {

     mode = argv[4]; 

  }

    vector<string> imageDims = (argc > 5 )? splitString(argv[5]) : splitString(inputDimensions);
    if (imageDims.size() != 4) {
        std::cout << "Invalid format! Accepted format: N, C, W, H" << std::endl;
        exit(1);
    }
    n = std::stoi(imageDims.at(0));
    c = std::stoi(imageDims.at(1));
    w = std::stoi(imageDims.at(2));
    h = std::stoi(imageDims.at(3));

    vector<string> kernelDims = (argc > 6 )? splitString(argv[6]) : splitString(kernelDimensions);
    if (kernelDims.size() != 4) {
        std::cout << "Invalid format! Accepted format: N, C, W, H" << std::endl;
        exit(1);
    }
    nk = std::stoi(kernelDims.at(0));
    ck = std::stoi(kernelDims.at(1));
    wk = std::stoi(kernelDims.at(2));
    hk = std::stoi(kernelDims.at(3));

    int stride = (argc > 7) ? atoi(argv[7]) : 1;

  std::cout << "Input dimensions: " << n << " " << c << " " << w << " " << h << std::endl;
  std::cout << "Kernel dimensions: " << nk << " " << ck << " " << wk << " " << hk << std::endl;
  std::cout << "Stride: " << stride << std::endl;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger =
      make_shared<pdb::PDBLogger>("Conv2DclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  string errMsg;
  string dbName = "conv2d";
  string img_set = "img";

  if (createSetOrNot == true) {
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
            dbName, img_set, errMsg, (size_t)512 * (size_t)1024 * (size_t)1024)) {
          cout << "Not able to create set " + img_set + ": " + errMsg;
      } else {
          cout << "Created set " << img_set << ".\n";
      }
  }

  //load data
  if (addDataOrNot == true) {
      load_rnd_img(w, h, c, n, numImages, pdbClient, dbName, img_set);
  }
 
  //create output dataset
  string feature_out_set = "feature_map";    
  pdbClient.removeSet(dbName, feature_out_set, errMsg);
  if (!pdbClient.createSet<pdb::TensorData>(
            dbName, feature_out_set, errMsg, (size_t)1024 * (size_t)1024 * (size_t)1024)) {
      cout << "Not able to create set " + feature_out_set + ": " + errMsg;
  } else {
      cout << "Created set " << feature_out_set << ".\n";
  }

  auto begin1 = std::chrono::high_resolution_clock::now();
  pdb::makeObjectAllocatorBlock(8 * 1024 * 1024, true);

  //create scan computation
  pdb::Handle<pdb::Computation> imageScanner =
      pdb::makeObject<pdb::ScanUserSet<pdb::TensorData>>(dbName, img_set);

  pdb::Handle<pdb::Vector<unsigned int>> dimensions = pdb::makeObject<pdb::Vector<unsigned int>>(4);
//   unsigned int x=1, y=1, z=64, n=64;
  dimensions->push_back(nk);
  dimensions->push_back(ck);
  dimensions->push_back(hk);
  dimensions->push_back(wk);

  pdb::Handle<TensorData> kernel = pdb::makeObject<TensorData>(4, dimensions);

  for (unsigned int i = 0; i < nk*ck*hk*wk; i++) {

     (*(kernel->rawData))[i] = 0.5;

  }


  //create select computation
  // stride (across all dimensions)
  pdb::Handle<pdb::Computation> select = pdb::makeObject<Conv2DSelect>(kernel, mode, stride);
  select->setInput(0, imageScanner);
  std::cout << "select's output type: "<< select->getOutputType() << std::endl;
  //create write computation
  pdb::Handle<pdb::Computation> myWriteSet =
      pdb::makeObject<pdb::WriteUserSet<pdb::TensorData>>(dbName, feature_out_set);
  myWriteSet->setInput(0, select);

  bool res = false;

  auto begin = std::chrono::high_resolution_clock::now();


  std::cout<< "---------------------------Starting execute computations---------------------------" << std::endl;
  // run the computation
  if (!pdbClient.executeComputations(errMsg, "conv2d-proj", myWriteSet)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  auto end = std::chrono::high_resolution_clock::now();

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
         //std::cout << count << ":" << (*(a->dimensions))[0] << "," << (*(a->dimensions))[1] << "," << (*(a->dimensions))[2] << ";";
     }
     std::cout << "output count:" << count << "\n";
  }



  //remove the output set
  pdbClient.removeSet(dbName, feature_out_set, errMsg);  

  std::cout << "Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  std::cout << "Translation Overhead: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(begin - begin1).count()
              << " secs." << std::endl;

  return 0;
}
