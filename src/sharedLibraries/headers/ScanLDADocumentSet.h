#ifndef SCAN_LDA_DOCUMENT_SET_H
#define SCAN_LDA_DOCUMENT_SET_H

#include "ScanUserSet.h"
#include "LDADocument.h"

/* The class for reading in LDA document */
using namespace pdb;
class ScanLDADocumentSet : public ScanUserSet<LDADocument> {

public:
    ENABLE_DEEP_COPY

    ScanLDADocumentSet() {}

    ScanLDADocumentSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
