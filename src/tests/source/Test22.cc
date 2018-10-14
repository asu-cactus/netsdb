
#ifndef TEST_22_CC
#define TEST_22_CC

#include "StorageClient.h"
#include "PDBVector.h"
#include "InterfaceFunctions.h"

// this won't be visible to the v-table map, since it is not in the built in types directory

int main(int argc, char* argv[]) {

    std::cout << "Make sure to run bin/test23 in a different window to provide a catalog/storage "
                 "server.\n";
    std::cout << "You can provide an argument as the database name, by default Test22_Database123 "
                 "will be used.\n";
    std::string databaseName("Test22_Database123");
    if (argc == 2) {
        databaseName = argv[1];
    }
    std::cout << "to add database with name: " << databaseName << std::endl;

    // start a client
    bool usePangea = true;
    pdb::StorageClient temp(8108, "localhost", make_shared<pdb::PDBLogger>("clientLog"), usePangea);
    string errMsg;

    // now, create a new database
    if (!temp.createDatabase(databaseName, errMsg)) {
        std::cout << "Not able to create database: " + errMsg;
        std::cout << "Please change a database name, or remove the pdbRoot AND CatalogDir "
                     "directories at where you run test23";
    } else {
        std::cout << "Created database.\n";
    }
}

#endif
