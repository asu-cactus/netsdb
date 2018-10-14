
#include <cstddef>
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <unistd.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include <string>

int main() {

    // for timing
    auto begin = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 10000; i++) {
        std::string* str = new std::string(
            "This is an object big enough to force flushing soon. This is an object big enough to "
            "force flushing soon. This is an object big enough to force flushing soon. This is an "
            "object big enough to force flushing soon. This is an object big enough to force "
            "flushing soon. This is an object big enough to force flushing soon. This is an object "
            "big enough to force flushing soon. This is an object big enough to force flushing "
            "soon. This is an object big enough to force flushing  soon. It has a total of 512 "
            "bytes to test. This is an object big enough to force flushing soon. This is an object "
            "big enough to force flushing soon. This is an object big enough to force flushing "
            "soon. This is an object big enough to force flushing soon. This is an object big "
            "enough to force flushing..");
        delete str;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Duration to create all of the std::string objects: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << " ns."
              << std::endl;
};
