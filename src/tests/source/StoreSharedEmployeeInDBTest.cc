#ifndef STORE_SHARED_EMPLOYEE_IN_DB_TEST_CC
#define STORE_SHARED_EMPLOYEE_IN_DB_TEST_CC

#include "StorageClient.h"
#include "PDBVector.h"
#include "Employee.h"
#include "InterfaceFunctions.h"

#include "SharedEmployee.h"

/**
 * This test stores lots of SharedEmployee Objects to the storage.
 */

int main(int argc, char* argv[]) {

    int numIterations = 1000;
    int numObjectInEachIterations = 100;


    // register the shared employee class
    pdb::StorageClient temp(8108, "localhost", make_shared<pdb::PDBLogger>("clientLog"), true);

    string errMsg;

    for (int num = 0; num < numIterations; ++num) {

        pdb::makeObjectAllocatorBlock(1024 * 1024 * 1, true);
        pdb::Handle<pdb::Vector<pdb::Handle<SharedEmployee>>> storeMe =
            pdb::makeObject<pdb::Vector<pdb::Handle<SharedEmployee>>>();

        // now, create a bunch of data
        // and store them in vector of 100 Objects
        try {

            for (int i = 0; i < numObjectInEachIterations; i++) {
                pdb::Handle<SharedEmployee> myData =
                    pdb::makeObject<SharedEmployee>("Joe Johnson" + to_string(i), i + 45);
                storeMe->push_back(myData);
            }

        } catch (pdb::NotEnoughSpace& n) {
            cout << "Out of Memory. We should not get here. " + errMsg;
        }

        // we got here, so go ahead and store the vector
        if (!temp.storeData<SharedEmployee>(storeMe, "chris_db", "chris_set", errMsg)) {
            cout << "Not able to store data: " + errMsg;
            return 0;
        }

        cout << "Stored " << num * numObjectInEachIterations << "  Objects." << endl;


    }  // End of ForLoop.
}
#endif
