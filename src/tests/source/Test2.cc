
#define NUM_OBJECTS 1000000

#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <unistd.h>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <fcntl.h>
#include <string>

#include "InterfaceFunctions.h"
#include "Employee.h"
#include "Supervisor.h"

using namespace pdb;

int main() {

    // for timing
    auto begin = std::chrono::high_resolution_clock::now();

    // create one million allocators and one million objects
    try {
        for (int i = 0; i < NUM_OBJECTS; i++) {
            if (i % 1000 == 0)
                std::cout << i << "\n";
            makeObjectAllocatorBlock(1024 * 24, true);
            Handle<Supervisor> super = makeObject<Supervisor>("Joe Johnson", 57);
            for (int j = 0; j < 10; j++) {
                Handle<Employee> temp = makeObject<Employee>("Steve Stevens", 57);
                super->addEmp(temp);
            }
        }

    } catch (NotEnoughSpace& e) {
        std::cout << "This is bad.  Why did I run out of RAM?\n";
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Duration to create all of the objects: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << " ns."
              << std::endl;
}
