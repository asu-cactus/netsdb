
#ifndef TABLE_C
#define TABLE_C

#include "MyDB_Table.h"

MyDB_Table::MyDB_Table(string name, string storageLocIn) {
    tableName = name;
    storageLoc = storageLocIn;
}

MyDB_Table::~MyDB_Table() {}

string& MyDB_Table::getName() {
    return tableName;
}

string& MyDB_Table::getStorageLoc() {
    return storageLoc;
}

#endif
