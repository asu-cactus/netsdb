
#ifndef TENSOR_BLOCK_IDENTIFIER_H
#define TENSOR_BLOCK_IDENTIFIER_H


#include "Object.h"
#include "Handle.h"
#include "DataTypes.h"
#include "PDBVector.h"

// PRELOAD %TensorBlockIdentifier%

namespace pdb {

// encapsulates a request to add a set in storage
class TensorBlockIdentifier : public Object {

public:
    ENABLE_DEEP_COPY

    TensorBlockIdentifier() {}
    ~TensorBlockIdentifier() {}

    TensorBlockIdentifier(DatabaseID dbId, UserTypeID typeId, SetID setId, std::vector<int> dimensionIndexes)
        : dbId(dbId), typeId(typeId), setId(setId) {
        this->dimensions = makeObject<Vector<int>>();
	for (int dim : dimensionIndexes) {
	    this->dimensions->push_back(dim);
	}
    }

    void print() {

        std::cout << "databaseID:" << dbId;
	std::cout << ", typeID:" << typeId;
        std::cout << ", setID:" << setId << std::endl;
	std::cout << "[ ";
        for (int i = 0; i < dimensions->size(); i++) {
	    std::cout << (*dimensions)[i] << " ";
	}
	std::cout << "]" << std::endl;
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

    int getNumDimensions() {
        return dimensions->size();
    }

    Handle<Vector<int>> getDimensions() {
        return dimensions;
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

    
    size_t hash() const override {
    
        size_t value = size_t(this->setId + 10 * typeId + 100 * dbId);
	for (int i = 0; i < dimensions->size(); i++) {
	    value = value + size_t((*dimensions)[i])*size_t(1000)*size_t(pow((size_t)(2), (size_t)(i)));
	}
	return value;	

    }

private:
    DatabaseID dbId;
    UserTypeID typeId;
    SetID setId;
    Handle<Vector<int>> dimensions=nullptr;
};
}

#endif
