/*
 * DistributedStorageCleanup.h
 *
 *  Created on: Nov 16, 2016
 *      Author: Jia
 */

#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_DISTRIBUTEDCLEANUP_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_DISTRIBUTEDCLEANUP_H_


#include "Object.h"

//  PRELOAD %DistributedStorageCleanup%

namespace pdb {
// this object type is sent to the server to tell it there is no more page to load, scan finished at
// frontend
class DistributedStorageCleanup : public pdb::Object {

public:
    DistributedStorageCleanup() {}
    ~DistributedStorageCleanup() {}

    ENABLE_DEEP_COPY
};
}

#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_DISTRIBUTEDCLEANUP_H_ */
