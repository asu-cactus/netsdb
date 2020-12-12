
#ifndef STRING_INT_PAIR_H
#define STRING_INT_PAIR_H

#include "Object.h"
#include "PDBString.h"
#include "Handle.h"

//  PRELOAD %StringIntPair%

namespace pdb {

class StringIntPair : public Object {

public:
    Handle<String> myString;
    int myInt;

    ENABLE_DEEP_COPY

    ~StringIntPair() {}
    StringIntPair() {}

    StringIntPair(std::string fromMe, int meTo) {
        myString = makeObject<String>(fromMe);
        myInt = meTo;
    }

    int getInt() {
        std::cout << "extract " << myInt << std::endl;
        return myInt;
    }

    int getSillyInt() {
        return myInt + myString->size();
    }

    Handle<String> getString() {
        std::cout << "extract " << myString->c_str() << std::endl;
        return myString;
    }

    Handle<String> getSillyString() {
        return makeObject<String>(std::string("Silly") + std::to_string(getSillyInt()));
    }


    StringIntPair getSillyPair() {
        return StringIntPair(std::string("Silly") + std::to_string(getSillyInt()),
                             myInt + myString->size());
    }

    bool operator==(const StringIntPair& toMe) {
        return ((myString == toMe.myString) && (myInt == toMe.myInt));
    }

    size_t hash() const override{
        return hashMe(myString->c_str(), myString->size());
    }

    std::string toString() override {
        return (std::string(myString->c_str()) + std::string("_") + std::to_string(myInt));
    }

    friend std::ostream& operator<<(std::ostream& stream, const StringIntPair& printMe) {
        return stream << "";
    }

};
}

#endif
