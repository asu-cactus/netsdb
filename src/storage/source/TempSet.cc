

#ifndef TEMP_SET_CC
#define TEMP_SET_CC

#include "TempSet.h"
#include "PartitionedFile.h"
#include "Configuration.h"
#include <vector>
using namespace std;


TempSet::TempSet(SetID setId,
                 string setName,
                 string metaTempPath,
                 vector<string> dataTempPaths,
                 SharedMemPtr shm,
                 PageCachePtr cache,
                 pdb::PDBLoggerPtr logger,
                 LocalityType localityType,
                 LocalitySetReplacementPolicy policy,
                 OperationType operation,
                 DurabilityType durability,
                 PersistenceType persistence,
                 size_t pageSize,
                 size_t desiredSize)
    : UserSet(logger,
              shm,
              0,
              0,
              0,
              setId,
              setName,
              cache,
              localityType,
              policy,
              operation,
              durability,
              persistence,
              pageSize,
              desiredSize) {
    vector<string> dataPaths;
    int numDataPaths = dataTempPaths.size();
    int i;
    string dataPath;
    PartitionedFilePtr file;
    for (i = 0; i < numDataPaths; i++) {
        dataPath = this->encodePath(dataTempPaths.at(i), setName);
        dataPaths.push_back(dataPath);
    }
    file = make_shared<PartitionedFile>(
        0, 0, 0, setId, this->encodePath(metaTempPath, setName), dataPaths, logger, pageSize);
    this->lastFlushedPageId = (unsigned int)(-1);  
    this->setFile(file);
    this->getFile()->openAll();
}


TempSet::~TempSet() {}


void TempSet::clear() {
    this->getFile()->clear();
}

string TempSet::encodePath(string tempPath, string setName) {
    char buffer[500];
    sprintf(buffer, "%s/%d_%s", tempPath.c_str(), setId, setName.c_str());
    return string(buffer);
}


#endif
