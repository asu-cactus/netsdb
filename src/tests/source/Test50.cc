
#ifndef TEST_50_H
#define TEST_50_H

#include "PDBString.h"
#include "Supervisor.h"
#include "PDBClient.h"

/* This test uses data and selection of builtInType to demonstrate a distributed query with
 * distributed storage */


using namespace pdb;
int main(int argc, char* argv[]) {

    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
    // Step 1. Create Database and Set

    PDBClient pdbClient(
            8108, "localhost",
            clientLogger,
            false,
            false);

    string errMsg;

    // now, create a new database
    if (!pdbClient.createDatabase("chris_db", errMsg)) {
        cout << "Not able to create database: " + errMsg;
        exit(-1);
    } else {
        cout << "Created database.\n";
    }

    // now, create a new set in that database
    if (!pdbClient.createSet("chris_db", "chris_set", "pdb::Supervisor", errMsg, DEFAULT_PAGE_SIZE)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }


    // now, create a new set in that database
    if (!pdbClient.createSet<pdb::Vector<pdb::Handle<pdb::Employee>>>(
            "chris_db", "output_set1", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }

    system("scripts/cleanupSoFiles.sh");
}

#endif
