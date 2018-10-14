
#ifndef PDB_COMM_WORK_C
#define PDB_COMM_WORK_C

#include "PDBCommWork.h"
#include <iostream>


namespace pdb {

PDBCommunicatorPtr PDBCommWork::getCommunicator() {
    return myCommunicator;
}

void PDBCommWork::setGuts(PDBCommunicatorPtr toMe) {
    myCommunicator = toMe;
}
}

#endif
