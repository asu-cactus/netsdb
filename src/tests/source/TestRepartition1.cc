#include "UserSet.h"
#include "JoinComp.h"
#include "RedditComment.h"
#include <cstddef>
#include <iostream>
#include <fstream>

using namespace pdb;



int main(int argc, char* argv[]) {

   std::string mapPath = argv[1];
   pdb::PDBLoggerPtr logger = make_shared<pdb::PDBLogger>("log.out");
   FILE * file = fopen(mapPath.c_str(), "r");
   size_t pageSize = (size_t)512*(size_t)1024*(size_t)1024;
   char * buffer = (char *) malloc (pageSize);
   size_t len = fread(buffer, sizeof(char), pageSize, file);
   Record<Vector<Handle<JoinMap<JoinTuple<reddit::Comment, char [0]>>>>> * myRecord = 
      (Record<Vector<Handle<JoinMap<JoinTuple<reddit::Comment, char [0]>>>>> *)(buffer);
   if (myRecord != nullptr) {
       Handle<Vector<Handle<JoinMap<JoinTuple<reddit::Comment, char [0]>>>>> myMaps =
           myRecord->getRootObject();
       std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
       for (int i = 0; i < myMaps->size(); i++) {   
           std::cout << i;
           std::cout << ": Got map: ";
           std::cout << (*myMaps)[i]->size() << std::endl; 
           (*myMaps)[i]->print();
       }
  }
  
   free(buffer);
   fclose(file);

}
