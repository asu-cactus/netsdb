#ifndef DISTRIBUTED_STORAGE_EXPORT_SET
#define DISTRIBUTED_STORAGE_EXPORT_SET

// by Jia, Apr 2017


#include "Object.h"
#include "PDBString.h"

namespace pdb {

// PRELOAD %DistributedStorageExportSet%
class DistributedStorageExportSet : public Object {

public:
    DistributedStorageExportSet() {}

    DistributedStorageExportSet(std::string dbName,
                                std::string setName,
                                std::string outputFilePath,
                                std::string format) {

        this->dbName = dbName;
        this->setName = setName;
        this->outputFilePath = outputFilePath;
        this->format = format;
    }

    std::string getDbName() {
        return dbName;
    }

    std::string getSetName() {
        return setName;
    }

    std::string getOutputFilePath() {
        return outputFilePath;
    }

    std::string getFormat() {
        return format;
    }

    ENABLE_DEEP_COPY


private:
    String dbName;

    String setName;

    String outputFilePath;

    String format;
};
}
#endif
