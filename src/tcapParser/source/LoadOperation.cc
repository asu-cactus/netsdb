#include "LoadOperation.h"

using std::make_shared;

namespace pdb_detail {
LoadOperation::LoadOperation(const string& source) : source(source) {}

void LoadOperation::match(function<void(LoadOperation&)> forLoad,
                          function<void(ApplyOperation&)>,
                          function<void(FilterOperation&)>,
                          function<void(HoistOperation&)>,
                          function<void(BinaryOperation&)>) {
    return forLoad(*this);
}
}