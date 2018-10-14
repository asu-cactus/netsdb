/*
 * CatalogStandardSetMetadata.h
 *
 *  Created on: Dec 8, 2016
 *      Author: carlos
 */

#ifndef SRC_CATALOG_CATALOGSTANDARDSETMETADATA_H_
#define SRC_CATALOG_CATALOGSTANDARDSETMETADATA_H_

#include <iostream>
#include <string>

using namespace std;

class CatalogStandardSetMetadata {
public:
  CatalogStandardSetMetadata();

  CatalogStandardSetMetadata(string pdbSetIdIn, string pdbSetKeyIn,
                             string pdbSetNameIn, string pdbDatabaseIdIn,
                             string pdbDatabaseNameIn, string pdbTypeIdIn,
                             string pdbTypeNameIn);

  CatalogStandardSetMetadata(const CatalogStandardSetMetadata &pdbSetToCopy);

  void setValues(string pdbSetIdIn, string pdbSetKeyIn, string pdbSetNameIn,
                 string pdbDatabaseIdIn, string pdbDatabaseNameIn,
                 string pdbTypeIdIn, string pdbTypeNameIn);

  ~CatalogStandardSetMetadata();

  // The set ID is used as key for storing metadata
  string getItemKey();

  string getItemId();

  string getDBId();

  string getObjectTypeId();

  string getItemName();

  string getDBName();

  string getObjectTypeName();

  void setTypeName(string &typeNameIn);

  void setDBName(string &pdbDatabaseNameIn);

  void setDBId(string &dbIdIn);

  void setTypeId(string &typeIdIn);

  void setItemKey(string &itemKeyIn);

  void setItemId(string &itemIdIn);

  void setItemName(string &itemNameIn);

  string printShort();

  friend std::ostream &operator<<(std::ostream &out,
                                  CatalogStandardSetMetadata &catalogSet) {
    out << "\nCatalogSet Metadata" << endl;
    out << "-------------------" << endl;
    out << "      DB Id: " << catalogSet.getDBId() << endl;
    out << "    DB name: " << catalogSet.getDBName() << endl;
    out << "     Set Id: " << catalogSet.getItemId() << endl;
    out << "    Set Key: " << catalogSet.getItemKey() << endl;
    out << "   Set Name: " << catalogSet.getItemName() << endl;
    out << "    Type Id: " << catalogSet.getObjectTypeId() << endl;
    out << "  Type Name: " << catalogSet.getObjectTypeName() << endl;
    out << "-------------------\n" << endl;
    return out;
  }

private:
  string pdbSetId;
  string setKey;
  string pdbSetName;

  string pdbDatabaseId;
  string pdbDatabaseName;

  string typeId;
  string typeName;
};

#endif /* SRC_CATALOG_CATALOGSTANDARDSETMETADATA_H_ */
