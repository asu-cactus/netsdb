/*
 * QueryPermitResponse.h
 *
 *  Created on: Apr 12, 2016
 *      Author: Kia
 */

#ifndef QUERY_PERMIT_RESPONSE_H
#define QUERY_PERMIT_RESPONSE_H

#include "Object.h"
#include "DataTypes.h"
#include "PDBString.h"


//  PRELOAD %QueryPermitResponse%


/**
 * This class encapsulates the permission request for running a query on distributed PDB cluster.
 *
 */
namespace pdb {

class QueryPermitResponse : public pdb::Object {
public:
    QueryPermitResponse() {}

    ~QueryPermitResponse() {}

    pdb::String& getQueryId() {
        return queryID;
    }

    void setQueryId(pdb::String& queryId) {
        this->queryID = queryId;
    }


private:
    pdb::String queryID;
};
}
#endif
