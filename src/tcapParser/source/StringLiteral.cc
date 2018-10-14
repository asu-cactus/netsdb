#include "StringLiteral.h"

#include <memory>

using std::make_shared;
using std::shared_ptr;

namespace pdb_detail {
StringLiteral::StringLiteral(const string& contents) : contents(contents) {}
}