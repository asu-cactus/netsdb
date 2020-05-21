//
// by Jia, Apr 2019
//

#include "PDBClient.h"
#include "RedditAuthor.h"

#include <string>
#include <random>
#include <chrono>

using namespace pdb;

void parseInputCSVFile(std::string fileName, int blockSizeInMB) {

  // the error message is put there
  string errMsg;
  std ::ifstream inFile(fileName.c_str());
  std :: string line;
  bool end = false;
  bool rollback = false;
  long total = 15000000;
  long sent = 0;
  pdb::makeObjectAllocatorBlock((size_t)blockSizeInMB * (size_t)1024 * (size_t)1024, true);
  pdb::Handle<pdb::Vector<pdb::Handle<reddit::Author>>> storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<reddit::Author>>> ();
  while (!end) {
      if (!rollback) {
          if(!std::getline(inFile, line)){
             end = true;
             break;
          }
      } 
      rollback = false;
      try {
          pdb::Handle<reddit::Author> author = pdb::makeObject<reddit::Author>(line);
          storeMe->push_back(author);
      }
      catch (pdb::NotEnoughSpace &n) {
          for (int pos = 0; pos < storeMe->size(); pos++) {
              size_t hashVal = Hasher<Handle<reddit::Author>>::hash((*storeMe)[pos]);
              std::cout << (*storeMe)[pos]->author << ": " << hashVal << std::endl;
          }

          rollback = true; 
          pdb::makeObjectAllocatorBlock((size_t)blockSizeInMB * (size_t)1024 * (size_t)1024, true);
          storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<reddit::Author>>> ();

      }
   }

}



int main(int argc, char* argv[]) {
 
  // we put the error here
  string errMsg;
  string inputFileName = argv[1];

  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
//  CatalogClient catalogClient(8108, "localhost", clientLogger);
//  PDBClient pdbClient(8108, "localhost", clientLogger, false, true);
//  pdbClient.registerType("libraries/libRedditAuthor.so", errMsg);

  
  parseInputCSVFile(inputFileName, 64);
}
