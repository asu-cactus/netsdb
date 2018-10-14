
#ifndef ENABLE_DEEP_COPY

// This defines the macro ENABLE_DEEP_COPY
//
// Any subclass of pdb :: Object that wishes to make proper use of the Object's
// deep copying facilities needs to have the macro ENABLE_DEEP_COPY somewhere in
// the public section of the class definition.  Example:
//
// class Foo : public Objct {
//
// public:
//
//     ENABLE_DEEP_COPY
//
// };
//
//
template <class ObjType>
void setUpAndCopyFromTemplate(void* target, void* source, const ObjType* dummy) {
    new (target) ObjType();
    *((ObjType*)target) = *((ObjType*)source);
}

template <class ObjType>
size_t computeSize(ObjType* dummy) {
    return sizeof(ObjType);
}

template <class ObjType>
void deleter(void* deleteMe, ObjType* dummy) {
    ((ObjType*)deleteMe)->~ObjType();
}

#define ENABLE_DEEP_COPY                                               \
    void setUpAndCopyFrom(void* target, void* source) const override { \
        setUpAndCopyFromTemplate(target, source, this);                \
    }                                                                  \
                                                                       \
    void deleteObject(void* deleteMe) override {                       \
        deleter(deleteMe, this);                                       \
    }                                                                  \
                                                                       \
    size_t getSize(void* ofMe) override {                              \
        return computeSize(this);                                      \
    }

#endif
