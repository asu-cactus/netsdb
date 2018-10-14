/*
 * CatalogPermissionsMetadata.h
 *
 *  Created on: Sept 12, 2016
 *      Author: carlos
 */

#ifndef CATALOG_PERMISSIONS_METADATA_H_
#define CATALOG_PERMISSIONS_METADATA_H_

#include <iostream>
#include "Object.h"
#include "PDBString.h"

//  PRELOAD %CatalogPermissionsMetadata%

using namespace std;

namespace pdb {

// This class serves to store information about users' permissions to databases in an
// instance of PDB.
// It also provides methods for maintaining their associated metadata.
// Clients of this class will access this information using a handler to the catalog.

class CatalogPermissionsMetadata : public Object {
public:
    ENABLE_DEEP_COPY

    CatalogPermissionsMetadata() {
    }

    ~CatalogPermissionsMetadata() {
    }
};

} /* namespace pdb */

#endif /* CATALOG_PERMISSIONS_METADATA_H_ */
