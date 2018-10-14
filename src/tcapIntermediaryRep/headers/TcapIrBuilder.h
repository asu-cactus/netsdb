#ifndef PDB_TCAPINTERMEDIARYREP_TCAPIRBUILDER_H
#define PDB_TCAPINTERMEDIARYREP_TCAPIRBUILDER_H

#include <algorithm>
#include <memory>
#include <vector>

#include "ApplyFunction.h"
#include "Filter.h"
#include "GreaterThan.h"
#include "Hoist.h"
#include "Instruction.h"
#include "Load.h"
#include "TcapParser.h"

using std::shared_ptr;
using std::vector;

namespace pdb_detail {
/**
 * Creates a list of instructions from the given parsed TCAP program.
 *
 * @param unit a parsed TCAP program
 * @return A correpsonding instruction representation.
 */
shared_ptr<vector<InstructionPtr>> buildTcapIr(TranslationUnit unit);
}

#endif  // PDB_TCAPINTERMEDIARYREP_TCAPIRBUILDER_H
