/*
 * QueryPermit.h
 *
 *  Created on: Mar 7, 2016
 *      Author: Kia
 */

#ifndef PLACE_OF_QUERY_PLANNER_H
#define PLACE_OF_QUERY_PLANNER_H

#include "Object.h"
#include "DataTypes.h"

//  PRELOAD %PlaceOfQueryPlanner%


/**
 * This class encapsulates the place of the query planner.
 *
 */
namespace pdb {

class PlaceOfQueryPlanner : public pdb::Object {

public:
    PlaceOfQueryPlanner() {}
    ~PlaceOfQueryPlanner() {}

    ENABLE_DEEP_COPY
private:
};
}

#endif
