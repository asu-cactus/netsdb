/*
 * CatalogStandardSetMetadata.cc
 *
 *  Created on: Dec 8, 2016
 *      Author: carlos
 */

#include "CatalogStandardSetMetadata.h"

using namespace std;

CatalogStandardSetMetadata::CatalogStandardSetMetadata() {}

CatalogStandardSetMetadata::CatalogStandardSetMetadata(
    string pdbSetIdIn, string pdbSetKeyIn, string pdbSetNameIn,
    string pdbDatabaseIdIn, string pdbDatabaseNameIn, string pdbTypeIdIn,
    string pdbTypeNameIn)
    : pdbSetId(pdbSetIdIn), setKey(pdbSetKeyIn), pdbSetName(pdbSetNameIn),
      pdbDatabaseId(pdbDatabaseIdIn), pdbDatabaseName(pdbDatabaseNameIn),
      typeId(pdbTypeIdIn), typeName(pdbTypeNameIn) {}

CatalogStandardSetMetadata::CatalogStandardSetMetadata(
    const CatalogStandardSetMetadata &pdbSetToCopy) {
  pdbSetId = pdbSetToCopy.pdbSetId;
  setKey = pdbSetToCopy.setKey;
  pdbSetName = pdbSetToCopy.pdbSetName;
  pdbDatabaseId = pdbSetToCopy.pdbDatabaseId;
  pdbDatabaseName = pdbSetToCopy.pdbDatabaseName;
  typeId = pdbSetToCopy.typeId;
  typeName = pdbSetToCopy.typeName;
}

void CatalogStandardSetMetadata::setValues(
    string pdbSetIdIn, string pdbSetKeyIn, string pdbSetNameIn,
    string pdbDatabaseIdIn, string pdbDatabaseNameIn, string pdbTypeIdIn,
    string pdbTypeNameIn) {

  pdbSetId = pdbSetIdIn;
  setKey = pdbSetKeyIn;
  pdbSetName = pdbSetNameIn;
  pdbDatabaseId = pdbDatabaseIdIn;
  pdbDatabaseName = pdbDatabaseNameIn;
  typeId = pdbTypeIdIn;
  typeName = pdbTypeNameIn;
}

CatalogStandardSetMetadata::~CatalogStandardSetMetadata() {}

// The set ID is used as key for storing metadata
string CatalogStandardSetMetadata::getItemKey() { return setKey; }

string CatalogStandardSetMetadata::getItemId() { return pdbSetId; }

string CatalogStandardSetMetadata::getDBId() { return pdbDatabaseId; }

string CatalogStandardSetMetadata::getObjectTypeId() { return typeId; }

string CatalogStandardSetMetadata::getItemName() { return pdbSetName; }

string CatalogStandardSetMetadata::getDBName() { return pdbDatabaseName; }

string CatalogStandardSetMetadata::getObjectTypeName() { return typeName; }

void CatalogStandardSetMetadata::CatalogStandardSetMetadata::setTypeName(
    string &typeNameIn) {
  typeName = typeNameIn;
}

void CatalogStandardSetMetadata::setDBName(string &pdbDatabaseNameIn) {
  pdbDatabaseName = pdbDatabaseNameIn;
}

void CatalogStandardSetMetadata::setDBId(string &dbIdIn) {
  pdbDatabaseId = dbIdIn;
}

void CatalogStandardSetMetadata::setTypeId(string &typeIdIn) {
  typeId = typeIdIn;
}

void CatalogStandardSetMetadata::setItemKey(string &itemKeyIn) {
  setKey = itemKeyIn;
}

void CatalogStandardSetMetadata::setItemId(string &itemIdIn) {
  pdbSetId = itemIdIn;
}

void CatalogStandardSetMetadata::setItemName(string &itemNameIn) {
  pdbSetName = itemNameIn;
}

string CatalogStandardSetMetadata::printShort() {
  string output;
  output = "   \nSet ";
  output.append(getItemId())
      .append(":")
      .append(getItemKey().c_str())
      .append(" | in DB ")
      .append(getDBId())
      .append(":")
      .append(getDBName().c_str())
      .append(" | for type ")
      .append(getObjectTypeId())
      .append(":")
      .append(getObjectTypeName().c_str());
  return output;
}
