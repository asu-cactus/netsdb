
#ifndef FUNC_TYPE_H
#define FUNC_TYPE_H

namespace pdb {

// there is one of these for each of the basic operations that can
// be encoded in a lambda
enum FuncType { UserDefined, GreaterThan, Equals, Plus };
}

#endif
