#include "BinaryOperation.h"

namespace pdb_detail {
void BinaryOperation::match(function<void(LoadOperation&)>,
                            function<void(ApplyOperation&)>,
                            function<void(FilterOperation&)>,
                            function<void(HoistOperation&)>,
                            function<void(BinaryOperation&)> forBinaryOp) {
    forBinaryOp(*this);
}
}