#include "MaterializationModeNamedSet.h"

namespace pdb_detail {

MaterializationModeNamedSet::MaterializationModeNamedSet(string databaseName, string setName)
    : _databaseName(databaseName), _setName(setName) {}

void MaterializationModeNamedSet::execute(MaterializationModeAlgo& algo) {
    algo.forNamedSet(*this);
}

bool MaterializationModeNamedSet::isNone() {
    return false;
}

string MaterializationModeNamedSet::tryGetDatabaseName(const string& defaultValue) {
    return getDatabaseName();
}

string MaterializationModeNamedSet::tryGetSetName(const string& defaultValue) {
    return getSetName();
}

string MaterializationModeNamedSet::getDatabaseName() {
    return _databaseName;
}

string MaterializationModeNamedSet::getSetName() {
    return _setName;
}
}