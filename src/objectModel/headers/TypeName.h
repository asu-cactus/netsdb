
#ifndef TYPE_NAME_H
#define TYPE_NAME_H

#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif

namespace pdb {

// returns the de-mangled name of the type T, as a string
template <class T>
std::string getTypeName();
}

#include "TypeName.cc"

#endif
