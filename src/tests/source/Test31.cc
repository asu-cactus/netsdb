
#ifndef TEST_31_CC
#define TEST_31_CC

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
    std::cout << "You can provide two argument as the name of the database to add, and name of the "
                 "set to add\n";
    std::cout << "By default Test31_Database123 will be used for database name, and Test31_Set123 "
                 "will be used for set name.\n";
    std::string databaseName("Test31_Database123");
    std::string setName("Test31_Set123");
    if (argc == 3) {
        databaseName = argv[1];
        setName = argv[2];
    }
    std::cout << "to add database with name: " << databaseName << std::endl;
    std::cout << "to add set with name: " << setName << std::endl;

    // start a client
    bool usePangea = true;
    pdb::StorageClient temp(8108, "localhost", make_shared<pdb::PDBLogger>("clientLog"), usePangea);
    string errMsg;


    // now, create a new database
    if (!temp.createDatabase(databaseName, errMsg)) {
        std::cout << "Not able to create database: " + errMsg << std::endl;
        std::cout << "Please change a database name, or remove the pdbRoot AND CatalogDir "
                     "directories at where you run test23"
                  << std::endl;
    } else {
        std::cout << "Created database.\n";
    }

    // now create a new set in that database
    if (!temp.createSet<SharedEmployee>(databaseName, setName, errMsg)) {
        std::cout << "Not able to set: " + errMsg << std::endl;
        std::cout << "Please change a set name, or remove the pdbRoot AND CatalogDir directories "
                     "at where you run test23"
                  << std::endl;
    } else {
        std::cout << "Created set.\n";
    }
}

#endif
