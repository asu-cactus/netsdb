
#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

#include "InterfaceFunctions.h"
#include "CatalogClient.h"
#include "PDBVector.h"
#include "Record.h"
#include "Handle.h"
#include "SharedEmployee.h"

using namespace pdb;

int main() {

    CatalogClient temp(8108, "localhost", std::make_shared<PDBLogger>("clientLog"));
    std::cout << "Make sure to run bin/test17 first to create a database file.\n";
    std::cout << "Make sure to run bin/test15 separately in a different window to get a catalog "
                 "server.\n";
    std::cout << "\tThe latter is needed to load the SharedLibEmployee class dynamically.\n";

    // load up the allocator with RAM
    makeObjectAllocatorBlock(1024 * 1024 * 24, false);

    // read in the first page in the file
    int filedesc = open("StorageDir/chris_db.chris_set", O_RDONLY);
    void* ram = malloc(1024 * 128);
    read(filedesc, ram, 1024 * 128);

    Record<Vector<Handle<SharedEmployee>>>* myData = (Record<Vector<Handle<SharedEmployee>>>*)ram;

    Handle<Vector<Handle<SharedEmployee>>> data = myData->getRootObject();
    std::cout << "Printing first 5 records on page from disk.\n";
    for (int i = 0; i < 5; i++) {
        (*data)[i]->print();
        std::cout << "\n";
    }

    close(filedesc);
    free(ram);
}
