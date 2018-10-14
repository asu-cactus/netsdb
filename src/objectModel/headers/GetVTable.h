
// this contains the GET_V_TABLE macro, which is used to create a shared library
// that can send an object type around the compute cluster

#include "UseTemporaryAllocationBlock.h"

#ifndef GET_V_TABLE
#define GET_V_TABLE(TYPE_NAME)                                                      \
                                                                                    \
    namespace pdb {                                                                 \
                                                                                    \
    /* this returns the name of the type that we are loading */                     \
    char typeName[300];                                                             \
                                                                                    \
    extern VTableMap* theVTable;                                                    \
    extern void* stackBase;                                                         \
    extern void* stackEnd;                                                          \
    extern bool inSharedLibrary;                                                    \
                                                                                    \
    extern "C" {                                                                    \
                                                                                    \
    char* getObjectTypeName() {                                                     \
        std::string typeString = getTypeName<TYPE_NAME>();                          \
        memmove(typeName, typeString.c_str(), typeString.size() + 1);               \
        return typeName;                                                            \
    }                                                                               \
                                                                                    \
    /* this returns an instance of the vtable of the type that we are loading */    \
    void* getObjectVTable() {                                                       \
        void* returnVal = nullptr;                                                  \
        const UseTemporaryAllocationBlock myBlock{1024 * 1024 * 4};                 \
        try {                                                                       \
            TYPE_NAME temp;                                                         \
            returnVal = temp.getVTablePtr();                                        \
                                                                                    \
        } catch (NotEnoughSpace & e) {                                              \
            std::cout << "Not enough memory to allocate TYPE_NAME";                 \
        }                                                                           \
        return returnVal;                                                           \
    }                                                                               \
                                                                                    \
    /* this must be called before getObjectVTable () to correctly set the static */ \
    /* and global variables that make the object model work correctly.  Since  */   \
    /* the shared library has *different* copies of all of these, they must be */   \
    /* set before the shared library is used to do anything.                   */   \
                                                                                    \
    void setAllGlobalVariables(Allocator* newAllocator,                             \
                               VTableMap* theVTableIn,                              \
                               void* stackBaseIn,                                   \
                               void* stackEndIn) {                                  \
        stackBase = stackBaseIn;                                                    \
        mainAllocatorPtr = newAllocator;                                            \
        stackEnd = stackEndIn;                                                      \
        theVTable = theVTableIn;                                                    \
        inSharedLibrary = true;                                                     \
    }                                                                               \
    }                                                                               \
    }

#endif
