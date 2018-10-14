
#ifndef JOIN_TESTS_H
#define JOIN_TESTS_H

#include "Handle.h"

// all of this nastiness allows us to call getSelection and getProjection on a join, using the
// correct number of args
namespace pdb {

extern GenericHandle foofoo;

struct HasTwoArgs {
    template <typename U>
    static auto test(U* x) -> decltype(x->getSelection(foofoo, foofoo)) {
        return x->getSelection(foofoo, foofoo);
    }
};

struct HasThreeArgs {
    template <typename U>
    static auto test(U* x) -> decltype(x->getSelection(foofoo, foofoo, foofoo)) {
        return x->getSelection(foofoo, foofoo, foofoo);
    }
};

struct HasFourArgs {
    template <typename U>
    static auto test(U* x) -> decltype(x->getSelection(foofoo, foofoo, foofoo, foofoo)) {
        return x->getSelection(foofoo, foofoo, foofoo, foofoo);
    }
};

struct HasFiveArgs {
    template <typename U>
    static auto test(U* x) -> decltype(x->getSelection(foofoo, foofoo, foofoo, foofoo, foofoo)) {
        return x->getSelection(foofoo, foofoo, foofoo, foofoo, foofoo);
    }
};

template <typename TypeToCallMethodOn>
auto callGetSelection(TypeToCallMethodOn& a, decltype(HasTwoArgs::test(&a)) * arg = nullptr) {
    GenericHandle first(1);
    GenericHandle second(2);
    return a.getSelection(first, second);
}

template <typename TypeToCallMethodOn>
auto callGetSelection(TypeToCallMethodOn& a, decltype(HasThreeArgs::test(&a)) * arg = nullptr) {
    GenericHandle first(1);
    GenericHandle second(2);
    GenericHandle third(3);
    return a.getSelection(first, second, third);
}

template <typename TypeToCallMethodOn>
auto callGetSelection(TypeToCallMethodOn& a, decltype(HasFourArgs::test(&a)) * arg = nullptr) {
    GenericHandle first(1);
    GenericHandle second(2);
    GenericHandle third(3);
    GenericHandle fourth(4);
    return a.getSelection(first, second, third, fourth);
}

template <typename TypeToCallMethodOn>
auto callGetSelection(TypeToCallMethodOn& a, decltype(HasFiveArgs::test(&a)) * arg = nullptr) {
    GenericHandle first(1);
    GenericHandle second(2);
    GenericHandle third(3);
    GenericHandle fourth(4);
    GenericHandle fifth(5);
    return a.getSelection(first, second, third, fourth, fifth);
}

template <typename TypeToCallMethodOn>
auto callGetProjection(TypeToCallMethodOn& a, decltype(HasTwoArgs::test(&a)) * arg = nullptr) {
    GenericHandle first(1);
    GenericHandle second(2);
    return a.getProjection(first, second);
}

template <typename TypeToCallMethodOn>
auto callGetProjection(TypeToCallMethodOn& a, decltype(HasThreeArgs::test(&a)) * arg = nullptr) {
    GenericHandle first(1);
    GenericHandle second(2);
    GenericHandle third(3);
    return a.getProjection(first, second, third);
}

template <typename TypeToCallMethodOn>
auto callGetProjection(TypeToCallMethodOn& a, decltype(HasFourArgs::test(&a)) * arg = nullptr) {
    GenericHandle first(1);
    GenericHandle second(2);
    GenericHandle third(3);
    GenericHandle fourth(4);
    return a.getProjection(first, second, third, fourth);
}

template <typename TypeToCallMethodOn>
auto callGetProjection(TypeToCallMethodOn& a, decltype(HasFiveArgs::test(&a)) * arg = nullptr) {
    GenericHandle first(1);
    GenericHandle second(2);
    GenericHandle third(3);
    GenericHandle fourth(4);
    GenericHandle fifth(5);
    return a.getProjection(first, second, third, fourth, fifth);
}
}

#endif
