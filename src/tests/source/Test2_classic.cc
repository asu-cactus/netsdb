
#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <chrono>
#include <cstring>
#include <Allocator.h>

using namespace pdb;

Allocator allocator;

class Employee {

    std::string *name{nullptr};
    int age;

public:

    ~Employee() {
        delete name;
    }

    Employee() = default;

    Employee(std::string &nameIn, int ageIn) {
        name = new std::string(nameIn);
        age = ageIn;
    }
};

class Supervisor {

private:
    Employee *me{nullptr};
    std::vector<Employee *> myGuys;

public:

    Supervisor() {
        for (auto a : myGuys) {
            delete a;
        }
    }

    ~Supervisor() = default;

    Supervisor(std::string name, int age) {
        me = new Employee(name, age);
    }

    void addEmp(Employee *addMe) {
        myGuys.push_back(addMe);
    }
};

int main() {

    // for timing
    auto begin = std::chrono::high_resolution_clock::now();

    // employee name
    std::string name = "Steve Stevens";

    // put a lot of copies of it into a vector
    for (int i = 0; i < 1000000; i++) {

        // create the supervisor
        Supervisor *mySup = new Supervisor("Joe Johnson", 23);

        for (int j = 0; j < 10; j++) {
            auto *temp = new Employee(name, 57);
            mySup->addEmp(temp);
        }

        delete mySup;
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Duration to create all of the objects: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << " ns."
              << std::endl;
}
