/*
 * QueryPermit.h
 *
 *  Created on: Mar 7, 2016
 *      Author: Kia
 */

#ifndef QUERY_PERMIT_H
#define QUERY_PERMIT_H

#include "Object.h"
#include "DataTypes.h"
#include "PDBString.h"


//  PRELOAD %QueryPermit%


/**
 * This class encapsulates the response to running a query request.
 *
 */

namespace pdb {

class QueryPermit : public pdb::Object {

public:
    QueryPermit() {}
    ~QueryPermit() {}

    ENABLE_DEEP_COPY

private:
};
}
#endif
