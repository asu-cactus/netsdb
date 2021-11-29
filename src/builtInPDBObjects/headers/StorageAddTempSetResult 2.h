#ifndef STORAGE_ADD_TEMP_SET_RESULT
#define STORAGE_ADD_TEMP_SET_RESULT

#include "SimpleRequestResult.h"
#include "DataTypes.h"

//  PRELOAD %StorageAddTempSetResult%


namespace pdb {

class StorageAddTempSetResult : public SimpleRequestResult {

public:
    StorageAddTempSetResult() {}
    ~StorageAddTempSetResult() {}

    StorageAddTempSetResult(bool res, std::string errMsg) : SimpleRequestResult(res, errMsg) {}

    StorageAddTempSetResult(bool res, std::string errMsg, SetID tempsetId)
        : SimpleRequestResult(res, errMsg) {
        this->tempsetId = tempsetId;
    }

    SetID getTempSetID() {
        return this->tempsetId;
    }
    void setTempSetID(SetID tempsetId) {
        this->tempsetId = tempsetId;
    }

    ENABLE_DEEP_COPY


private:
    SetID tempsetId;
};
}


#endif
