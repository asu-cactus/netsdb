
#ifndef PTR_H
#define PTR_H

namespace pdb {

class PtrBase {};

// this class wraps up a pointer, and provides implicity dereferencing...
// it is used during query processing, whenever we want to store a pointer
// to a data member.  This will also be used when we have a C++ lambda or
// a method call that returns a reference; in this case, we store a pointer
// to the value so that we can avoid a copy
template <class BaseType>
class Ptr : public PtrBase {

private:
    BaseType* data;

public:
    Ptr() {
        data = nullptr;
    }

    Ptr(BaseType* fromMe) {
        data = fromMe;
    }

    Ptr(BaseType& fromMe) {
        data = &fromMe;
    }

    Ptr& operator=(BaseType* fromMe) {
        data = fromMe;
        return *this;
    }

    Ptr& operator=(BaseType& fromMe) {
        data = &fromMe;
        return *this;
    }

    ~Ptr() {}

    operator BaseType&() const {
        return *data;
    }

    operator BaseType*() const {
        return data;
    }

    static size_t getObjectSize() {
        return sizeof(BaseType);
    }
};
}

#endif
