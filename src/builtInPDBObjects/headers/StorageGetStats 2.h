
#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_GETSTATS_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_GETSTATS_H_

#include "Object.h"
#include "DataTypes.h"

//  PRELOAD %StorageGetStats%

namespace pdb {
// this object type is sent to the server to tell it to print all stats
class StorageGetStats : public pdb::Object {


public:
    StorageGetStats() {}
    ~StorageGetStats() {}

    ENABLE_DEEP_COPY

};
}


#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_GETSTATS_H_ */
