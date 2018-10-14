/*
 * CatalogCloseSQLiteDBHandler.h
 *
 *  Created on: Jan 3, 2017
 *      Author: carlos
 */

#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_CATALOGCLOSESQLITEDBHANDLER_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_CATALOGCLOSESQLITEDBHANDLER_H_

#include "Object.h"

//  PRELOAD %CatalogCloseSQLiteDBHandler%

using namespace std;

namespace pdb {

// Encapsulates a request for closing the SQLite DB Handler
class CatalogCloseSQLiteDBHandler : public Object {
public:
    CatalogCloseSQLiteDBHandler() {}

    ENABLE_DEEP_COPY
};

} /* namespace pdb */

#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_CATALOGCLOSESQLITEDBHANDLER_H_ */
