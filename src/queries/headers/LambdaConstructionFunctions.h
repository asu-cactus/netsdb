
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <memory>
#include "Lambda.h"
#include "LambdaTypes.h"

namespace pdb {

// checks for equality
template <class L, class R>
Lambda<bool> operator==(Lambda<L> left, Lambda<R> right) {
    return Lambda<bool>(std::make_shared<EqualsOp<L, R>>(left, right));
}

// checks if the LHS is greater than the right
template <class L, class R>
Lambda<bool> operator<(Lambda<L> left, Lambda<R> right) {
    return Lambda<bool>(std::make_shared<GreaterThanOp<L, R>>(left, right));
}

// adds two values
template <class T>
Lambda<T> operator+(Lambda<T> left, Lambda<T> right) {
    return Lambda<T>(std::make_shared<PlusOp<T>>(left, right));
}
}

#endif
