
#include <cstddef>
#include <sstream>
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


#include "InterfaceFunctions.h"
#include "Employee.h"

using namespace pdb;

int main() {

    // for timing
    auto begin = std::chrono::high_resolution_clock::now();

    // load up the allocator with RAM
    makeObjectAllocatorBlock(1024 * 1024 * 24, false);

    Handle<Vector<Vector<Employee>>> data = makeObject<Vector<Vector<Employee>>>();

    for (int i = 0; i < 100; i++) {

        Vector<Employee> temp;
        for (int j = 0; j < 100; j++) {
            std::stringstream ss;
            ss << "myEmployee " << i << ", " << j;
            Employee temp2(ss.str(), j);
            temp.push_back(temp2);
        }

        data->push_back(temp);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Duration to create all of the objects: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << " ns."
              << std::endl;

    std::cout << "Are " << getBytesAvailableInCurrentAllocatorBlock()
              << " bytes left in the current allocation block.\n";

    Record<Vector<Vector<Employee>>>* myBytes = getRecord<Vector<Vector<Employee>>>(data);
    int filedesc = open("testfile6", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    size_t sizeWritten = write(filedesc, myBytes, myBytes->numBytes());
    if (sizeWritten == 0) {
        std::cout << "Write failed" << std::endl;
    }
    close(filedesc);
}
