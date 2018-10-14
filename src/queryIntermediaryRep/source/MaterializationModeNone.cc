#include "MaterializationModeNone.h"

namespace pdb_detail {
bool MaterializationModeNone::isNone() {
    return true;
}

void MaterializationModeNone::execute(MaterializationModeAlgo& algo) {
    algo.forNone(*this);
}

string MaterializationModeNone::tryGetDatabaseName(const string& noneValue) {
    return noneValue;
}

string MaterializationModeNone::tryGetSetName(const string& noneValue) {
    return noneValue;
}
}