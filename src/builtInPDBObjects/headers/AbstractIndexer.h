#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_ABSTRACTINDEXER_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_ABSTRACTINDEXER_H_


#include "Object.h"
#include "Handle.h"
#include <memory>
#include "DataTypes.h"
#include "PDBPage.h"

#include <iostream>

//  PRELOAD %AbstractIndexer%

namespace pdb {

class AbstractIndexer : public Object {

public:
  ENABLE_DEEP_COPY

    AbstractIndexer() {}
    ~AbstractIndexer() {}

    virtual void initialize() {
        std::cout << "Called AbstractIndexer initialize: " << std::endl;
    }

    virtual void addPage(PDBPagePtr page) {
        std::cout << "Called AbstractIndexer addPage: " << std::endl;
    }

    virtual SharedPageID* checkAndAddPage(PDBPagePtr page) {
        std::cout << "Called AbstractIndexer checkAndAddPage: " << std::endl;
        return nullptr;
    }

    virtual SharedPageID* queryPage(PDBPagePtr page) {
        return nullptr;
    }

    virtual void dump() {
        std::cout << "Called AbstractIndexer dump: " << std::endl;
    }
};
}

#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_ABSTRACTINDEXER_H_ */
