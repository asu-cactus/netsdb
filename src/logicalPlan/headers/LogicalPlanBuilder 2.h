#ifndef _LOGICALPLANBUILDER_H_
#define _LOGICALPLANBUILDER_H_

#include <memory>
#include <vector>

#include "Instruction.h"
#include "ParserTypes.h"
#include "SafeResult.h"

using std::shared_ptr;
using std::string;
using std::vector;

using pdb::SafeResult;

using pdb_detail::InstructionPtr;

/**
 * Translates each of the given instructions into an instance one of: Input, Output, Computation
 * and agregates the results into the returned LogicalPlan.
 *
 * Load turns into Input
 * Store turns into Output
 * everything other instruction variant turns into a Computation.
 *
 * @param instructions the list of instructions to translate
 * @return A LogicalPlan representation of the given instructions or a failure state if the
 * translation failed.
 */
shared_ptr<SafeResult<LogicalPlan>> buildLogicalPlan(
    shared_ptr<vector<InstructionPtr>> instructions);

/**
 * Translates the given TCAP program into an equivalent LogicalPlan.
 *
 * @param tcapProgram the TCAP program to translate.
 * @return A LogicalPlan representation of the given program or a failure state if the translation
 * failed.
 */
shared_ptr<SafeResult<LogicalPlan>> buildLogicalPlan(string tcapProgram);

#endif