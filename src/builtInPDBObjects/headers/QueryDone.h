/*
 * QueryPermit.h
 *
 *  Created on: Mar 7, 2016
 *      Author: Kia
 */

#ifndef QUERY_DONE_H
#define QUERY_DONE_H

#include "Object.h"
#include "DataTypes.h"
#include "PDBString.h"

//  PRELOAD %QueryDone%

/**
 * This message is sent when the query executor node is done with processing of query.
 */
namespace pdb {

class QueryDone : public pdb::Object {

public:
    QueryDone() {}
    ~QueryDone() {}

    pdb::String& getQuerID() {
        return queryID;
    }

    void setQueryID(pdb::String& queryID) {
        this->queryID = queryID;
    }

    ENABLE_DEEP_COPY

private:
    pdb::String queryID;
};
}
#endif
