
#include "SourceSetNameIr.h"

namespace pdb_detail {
SourceSetNameIr::SourceSetNameIr() {}

SourceSetNameIr::SourceSetNameIr(string databaseName, string setName)
    : _databaseName(databaseName), _setName(setName) {}

void SourceSetNameIr::execute(SetExpressionIrAlgo& algo) {
    algo.forSourceSetName(*this);
}

string SourceSetNameIr::getName() {
    return "SourceSetNameIr";
}

string SourceSetNameIr::getDatabaseName() {
    return _databaseName;
}


string SourceSetNameIr::getSetName() {
    return _setName;
}
}
