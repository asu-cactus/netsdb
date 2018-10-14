#ifndef PDB_TCAPPARSER_TCAPPARSER_H
#define PDB_TCAPPARSER_TCAPPARSER_H

#include <memory>
#include <string>

#include "SafeResult.h"
#include "TranslationUnit.h"

using std::shared_ptr;
using std::string;

using pdb::SafeResult;

namespace pdb_detail {
/**
 * Parses the given TCAP program string into a TranslationUnit.
 *
 * @param source a program in the TCAP language
 * @return SafeResultSuccess if the string parsed, else a SafeResultFailure.
 */
shared_ptr<SafeResult<TranslationUnit>> parseTcap(const string& source);
}

#endif  // PDB_TCAPPARSER_TCAPPARSER_H
