/*
 * CatalogStandardUserTypeMetadata.cc
 *
 *  Created on: Dec 8, 2016
 *      Author: carlos
 */

#include "CatalogStandardUserTypeMetadata.h"

using namespace std;

CatalogStandardUserTypeMetadata::CatalogStandardUserTypeMetadata() {}

CatalogStandardUserTypeMetadata::CatalogStandardUserTypeMetadata(
    string itemIdIn, string objectIDIn, string objectTypeIn,
    string objectNameIn, string libraryBytesIn)
    : itemId(itemIdIn), objectID(objectIDIn), objectType(objectTypeIn),
      objectName(objectNameIn), libraryBytes(libraryBytesIn) {}

CatalogStandardUserTypeMetadata::CatalogStandardUserTypeMetadata(
    const CatalogStandardUserTypeMetadata &pdbCatalogEntryToCopy) {
  itemId = pdbCatalogEntryToCopy.itemId;
  objectID = pdbCatalogEntryToCopy.objectID;
  objectType = pdbCatalogEntryToCopy.objectType;
  objectName = pdbCatalogEntryToCopy.objectName;
  libraryBytes = pdbCatalogEntryToCopy.libraryBytes;
}

CatalogStandardUserTypeMetadata::~CatalogStandardUserTypeMetadata() {}

string CatalogStandardUserTypeMetadata::getItemKey() { return objectName; }

string CatalogStandardUserTypeMetadata::getItemId() { return itemId; }

string CatalogStandardUserTypeMetadata::getObjectID() { return objectID; }

string CatalogStandardUserTypeMetadata::getObjectType() { return objectType; }

string CatalogStandardUserTypeMetadata::getItemName() { return objectName; }

string CatalogStandardUserTypeMetadata::getLibraryBytes() {
  return libraryBytes;
}

void CatalogStandardUserTypeMetadata::setObjectId(string &objectIdIn) {
  objectID = objectIdIn;
}

void CatalogStandardUserTypeMetadata::setItemId(string &itemIdIn) {
  itemId = itemIdIn;
}

void CatalogStandardUserTypeMetadata::setItemKey(string &itemKeyIn) {
  objectName = itemKeyIn;
}

void CatalogStandardUserTypeMetadata::setItemName(string &itemIdIn) {
  objectName = itemIdIn;
}

void CatalogStandardUserTypeMetadata::setLibraryBytes(string &bytesIn) {
  libraryBytes = bytesIn;
}

string CatalogStandardUserTypeMetadata::printShort() {
  string output;
  output = "   Type ID: ";
  output.append(getObjectID().c_str())
      .append(" | Type Name: ")
      .append(getItemName().c_str());
  return output;
}
