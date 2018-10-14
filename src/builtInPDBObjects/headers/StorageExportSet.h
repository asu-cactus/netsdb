#ifndef STORAGE_EXPORT_SET
#define STORAGE_EXPORT_SET


#include "Object.h"
#include "PDBString.h"

namespace pdb {

// PRELOAD %StorageExportSet%
class StorageExportSet : public Object {

public:
    StorageExportSet() {}

    StorageExportSet(std::string dbName,
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
