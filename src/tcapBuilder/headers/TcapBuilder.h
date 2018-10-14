#ifndef PDB_TCAPBUILDER_TCAPBUILDER_H
#define PDB_TCAPBUILDER_TCAPBUILDER_H

#include <string>

#include "QueryBase.h"

using std::string;

using pdb::QueryBaseHdl;

namespace pdb_detail {
string buildTcap(QueryBaseHdl base);
}

#endif  // PDB_TCAPBUILDER_TCAPBUILDER_H
