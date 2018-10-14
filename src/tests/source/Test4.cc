
#define NUM_OBJECTS 10371

#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

#include "InterfaceFunctions.h"
#include "Employee.h"
#include "Supervisor.h"

using namespace pdb;

int main() {

    // for timing
    auto begin = std::chrono::high_resolution_clock::now();

    // load up the allocator with RAM
    makeObjectAllocatorBlock(1024 * 1024 * 24, true);

    Handle<Vector<Handle<Supervisor>>> supers = makeObject<Vector<Handle<Supervisor>>>();
    int i = 0;
    try {
        // put a lot of copies of it into a vector
        for (; true; i++) {

            std::cout << i << "\n";
            Handle<Supervisor> super = makeObject<Supervisor>("Joe Johnson", 20 + (i % 29));
            supers->push_back(super);
            for (int j = 0; j < 10; j++) {
                Handle<Employee> temp = makeObject<Employee>("Steve Stevens", 20 + ((i + j) % 29));
                (*supers)[i]->addEmp(temp);
            }
            if (i > NUM_OBJECTS) {
                break;
            }
        }

    } catch (NotEnoughSpace& e) {
        std::cout << "Finally, after " << i << " inserts, ran out of RAM for the objects.\n";
    }

    // print the first ten items
    std::cout << "The first 10 items are: \n";
    for (int i = 0; i < 10; i++) {
        (*supers)[i]->print();
    }

    // get every thousandth item
    std::cout << "\nPrinting every 1000th item: \n";
    for (int i = 0; i < supers->size(); i += 1000) {
        (*supers)[i]->print();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Duration to create all of the objects and do the manipulations: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << " ns."
              << std::endl;

    std::cout << "Now writing the linked list to a file.\n";
    begin = std::chrono::high_resolution_clock::now();
    Record<Vector<Handle<Supervisor>>>* myBytes = getRecord<Vector<Handle<Supervisor>>>(supers);
    int filedesc = open("testfile3", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    size_t sizeWritten = write(filedesc, myBytes, myBytes->numBytes());
    if (sizeWritten == 0) {
        std::cout << "Write failed." << std::endl;
    }
    close(filedesc);

    end = std::chrono::high_resolution_clock::now();
    std::cout << "Duration to write the objects to a file: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << " ns."
              << std::endl;

    std::cout << "Wrote " << myBytes->numBytes() << " bytes to the file.\n";
}
