
#ifndef STORAGE_GET_DATA_RESPONSE_H
#define STORAGE_GET_DATA_RESPONSE_H

#include "Object.h"
#include "Handle.h"
#include <utility>

// PRELOAD %StorageGetDataResponse%

namespace pdb {

// encapsulates a response for request to obtain data from a storage set
class StorageGetDataResponse : public Object {

public:
    StorageGetDataResponse(){};
    ~StorageGetDataResponse(){};
    StorageGetDataResponse(int numPages,
                           std::string databaseName,
                           std::string setName,
                           size_t rawPageSize,
                           size_t pageSize,
                           bool success,
                           std::string errMsg)
        : numPages(numPages),
          databaseName(databaseName),
          setName(setName),
          rawPageSize(rawPageSize),
          pageSize(pageSize),
          errMsg(errMsg),
          success(success) {}

    int getNumPages() {
        return numPages;
    }

    std::string getDatabaseName() {
        return databaseName;
    }

    std::string getSetName() {
        return setName;
    }

    std::pair<bool, std::string> wasSuccessful() {
        return std::make_pair(success, errMsg);
    }

    size_t getRawPageSize() {
        return rawPageSize;
    }

    size_t getPageSize() {
        return pageSize;
    }

    ENABLE_DEEP_COPY

private:
    int numPages;
    std::string databaseName;
    std::string setName;
    size_t rawPageSize;
    size_t pageSize;
    String errMsg;
    bool success;
};
}

#endif
