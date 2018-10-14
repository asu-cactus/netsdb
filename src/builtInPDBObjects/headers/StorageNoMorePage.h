
#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_NOMOREPAGE_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_NOMOREPAGE_H_


#include "Object.h"

//  PRELOAD %StorageNoMorePage%

namespace pdb {
// this object type is sent to the server to tell it there is no more page to load, scan finished at
// frontend
class StorageNoMorePage : public pdb::Object {

public:
    StorageNoMorePage() {}
    ~StorageNoMorePage() {}

    ENABLE_DEEP_COPY
};
}

#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_NOMOREPAGE_H_ */
