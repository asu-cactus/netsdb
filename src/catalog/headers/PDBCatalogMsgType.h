/*
 * File:   PDBCatalogMsgType.h
 * Author: carlos
 *
 */

#ifndef PDBCATALOGMSGTYPE_H
#define PDBCATALOGMSGTYPE_H

// this lists all of the identifiers for the different types of Catalog Related
// Metadata

enum PDBCatalogMsgType {
  CatalogPDBNode, // 0

  CatalogPDBRegisteredObject, // 1

  CatalogPDBDatabase, // 2

  CatalogPDBUser, // 3

  CatalogGetPDBRegisteredObject, // 4

  GetSerializedCatalog, // 5

  SerializedCatalog, // 6

  GetCatalogMetadataAsString, // 7

  CatalogPDBSet, // 8

  CatalogPDBPermissions, // 9

  CatalogDataTypeId // 10

};

#endif /* PDBCATALOGMSGTYPE_H */
