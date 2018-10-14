
#define NUM_OBJECTS 12000

#include <cstddef>
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <unistd.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include <string>
#include <fstream>
#include "Handle.h"
#include "PDBVector.h"
#include "InterfaceFunctions.h"
#include "JoinMap.h"
#include "Configuration.h"
using namespace pdb;

int main(int argc, char* argv[]) {

    std::string fileName = argv[1];
    std::string type = argv[2];
    FILE* myFile = fopen(fileName.c_str(), "r");
    char* buffer = (char*)malloc(DEFAULT_NET_PAGE_SIZE * sizeof(char));
    fread(buffer, 1, DEFAULT_NET_PAGE_SIZE, myFile);
    if (type == "broadcast") {
        Handle<JoinMap<Object>> myMap = ((Record<JoinMap<Object>>*)buffer)->getRootObject();
        std::cout << "broadcasted map size is " << myMap->size() << std::endl;
    } else {
        Handle<Vector<Handle<JoinMap<Object>>>> myMaps =
            ((Record<Vector<Handle<JoinMap<Object>>>>*)buffer)->getRootObject();
        std::cout << "number of partitioned maps is " << myMaps->size() << std::endl;
        for (int i = 0; i < myMaps->size(); i++) {
            std::cout << "the " << i << "-th partitioned map size is " << (*myMaps)[i]->size()
                      << std::endl;
        }
    }
}
