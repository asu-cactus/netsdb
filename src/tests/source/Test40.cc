
#ifndef TEST_36_CC
#define TEST_36_CC

#include "PDBString.h"
#include "PDBMap.h"
#include "Employee.h"
#include "InterfaceFunctions.h"

#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cassert>

using namespace pdb;

class Foo : public Object {

public:
    Handle<Map<String, String>> myMap;

    Foo() {
        myMap = makeObject<Map<String, String>>();
        String temp1("this is fun");
        String temp2("so is this");
        (*myMap)[temp1] = temp2;
    }

    ENABLE_DEEP_COPY
};

int main() {

    makeObjectAllocatorBlock(124 * 1024 * 1024, true);

    Map<int, Handle<Foo>> myMap;
    for (int i = 0; i < 1000; i++) {
        Handle<Foo> temp = makeObject<Foo>();
        myMap[i] = temp;
    }
    assert(myMap.size() == 1000);

    Handle<Map<int, Handle<int>>> anotherMap = makeObject<Map<int, Handle<int>>>();
    for (int i = 0; i < 1000; i++) {
        Handle<int> temp = makeObject<int>();
        *temp = i;
        (*anotherMap)[i] = temp;
    }
    assert(anotherMap->size() == 1000);

    for (auto& a : *anotherMap) {
        std::cout << "(" << a.key << ", " << *(a.value) << ") ";
    }

    /*	Vector <Handle <Foo>> myVec;
        for (int i = 0; i < 1000; i++) {
            Handle <Foo> temp = makeObject <Foo> ();
            myVec.push_back (temp);
        }

        Vector <Handle <Foo> > myVec2 (1000, 1000);
        for (int i = 0; i < 1000; i++) {
            myVec2 [i] = myVec [i];
        }
    */
}


#endif
