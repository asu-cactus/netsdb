
#ifndef SET_IDENTIFIER_H
#define SET_IDENTIFIER_H


#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "DataTypes.h"

// PRELOAD %SetIdentifier%

namespace pdb {

// encapsulates a request to add a set in storage
class SetIdentifier : public Object {

public:
    ENABLE_DEEP_COPY

    SetIdentifier() {}
    ~SetIdentifier() {}

    SetIdentifier(std::string dataBase, std::string setName)
        : dataBase(dataBase), setName(setName) {
        setType = UserSetType;
        isAggregationResultOrNot = false;
    }

    SetIdentifier(std::string dataBase,
                  std::string setName,
                  SetType setType,
                  bool isProbingAggregationResult)
        : dataBase(dataBase),
          setName(setName),
          setType(setType),
          isAggregationResultOrNot(isProbingAggregationResult) {}


    std::string getDatabase() {
        return dataBase;
    }

    std::string getSetName() {
        return setName;
    }

    void setDatabaseId(DatabaseID dbId) {
        this->dbId = dbId;
    }

    void setTypeId(UserTypeID typeId) {
        this->typeId = typeId;
    }

    void setSetId(SetID setId) {
        this->setId = setId;
    }

    DatabaseID getDatabaseId() {
        return dbId;
    }

    UserTypeID getTypeId() {
        return typeId;
    }

    SetID getSetId() {
        return setId;
    }

    SetType getSetType() {
        return setType;
    }


    bool isAggregationResult() {
        return this->isAggregationResultOrNot;
    }

    void setNumPages(size_t numPages) {
        this->numPages = numPages;
    }

    size_t getNumPages() {
        return this->numPages;
    }

    void setPageSize(size_t pageSize) {
        this->pageSize = pageSize;
    }

    size_t getPageSize() {
        return this->pageSize;
    }

    void setIndexInInputs (int indexInInputs) {
        this->indexInInputs = indexInInputs;
    }

    int getIndexInInputs () {
        return this->indexInInputs;
    }

    void setDataType (std::string dataType) {
        this->dataType = dataType;
    }

    std::string getDataType () {
        return this->dataType;
    }

    void setNumHashKeys (int numHashKeys) {
        this->numHashKeys = numHashKeys;
    }

    int getNumHashKeys () {
        return this->numHashKeys;
    }

    void setDesiredSize (size_t desiredSize) {
        this->desiredSize = desiredSize;
    }

    size_t getDesiredSize () {
        return this->desiredSize;
    }

    void setDatabase(std::string database){
        dataBase = database;

    }

    void setSetName (std::string setName) {
        setName = setName;
    }

private:
    String dataBase;
    String setName;
    DatabaseID dbId;
    UserTypeID typeId;
    SetID setId;
    SetType setType;
    bool isAggregationResultOrNot;
    size_t numPages;
    size_t pageSize;
    int indexInInputs = 0;
    int numHashKeys = 0;
    String dataType;
    size_t desiredSize;
};
}

#endif
