
#ifndef SET_SPECIFIER_H
#define SET_SPECIFIER_H

// by Jia, Oct 2016

#include <string>
#include "DataTypes.h"
#include "Configuration.h"
#include <memory>
class SetSpecifier;
typedef std::shared_ptr<SetSpecifier> SetSpecifierPtr;


// encapsulates a request to add a set in storage
class SetSpecifier {

public:
    SetSpecifier() {}
    ~SetSpecifier() {}

    SetSpecifier(std::string dataBase,
                 std::string setName,
                 DatabaseID dbId,
                 UserTypeID typeId,
                 SetID setId,
                 size_t pageSize = DEFAULT_PAGE_SIZE)
        : dataBase(dataBase),
          setName(setName),
          dbId(dbId),
          typeId(typeId),
          setId(setId),
          pageSize(pageSize) {}

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

    void setPageSize(size_t pageSize) {
        this->pageSize = pageSize;
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

    size_t getPageSize() {
        return pageSize;
    }


private:
    std::string dataBase;
    std::string setName;
    DatabaseID dbId;
    UserTypeID typeId;
    SetID setId;
    size_t pageSize;
};


#endif
