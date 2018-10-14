
#ifndef TEST_33_CC
#define TEST_33_CC

#include "StorageClient.h"
#include "PDBVector.h"
#include "InterfaceFunctions.h"
#include "SharedEmployee.h"

// this won't be visible to the v-table map, since it is not in the built in types directory

int main(int argc, char* argv[]) {

    std::cout << "Firstly: Make sure to run bin/test23 or bin/test28 in a different window to "
                 "provide a catalog/storage server.\n";
    std::cout << "Secondly: Make sure to run bin/test24 to register the SharedEmployee type once "
                 "and only once.\n";
    std::cout << "Thirdly: Make sure to run bin/test31 to create the destination set.\n";
    std::string databaseName("Test31_Database123");
    std::string setName("Test31_Set123");
    std::cout << "to remove database with name: " << databaseName << std::endl;
    std::cout << "to remove set with name: " << setName << std::endl;

    // start a client
    bool usePangea = true;
    pdb::StorageClient temp(8108, "localhost", make_shared<pdb::PDBLogger>("clientLog"), usePangea);
    string errMsg;


    // now remove the destination set
    if (!temp.removeSet<SharedEmployee>(databaseName, setName, errMsg)) {
        std::cout << "Not able to remove set: " + errMsg << std::endl;
        std::cout << "Please change a set name, or remove the pdbRoot AND CatalogDir directories "
                     "at where you run test23/test28"
                  << std::endl;
    } else {
        std::cout << "Removed set.\n";
    }
}

#endif
