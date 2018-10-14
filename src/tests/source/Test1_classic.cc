
#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iterator>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <chrono>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>

class Employee {
    std::string* name;
    int age;

public:
    ~Employee() {
        delete name;
    }

    Employee() {}

    Employee(std::string nameIn, int ageIn) {
        name = new std::string(nameIn);
        age = ageIn;
    }
};

class Supervisor {

private:
    Employee* me;
    std::vector<Employee*> myGuys;

public:
    ~Supervisor() {
        delete me;
        for (auto a : myGuys) {
            delete a;
        }
    }

    Supervisor() {}

    Supervisor(std::string name, int age) {
        me = new Employee(name, age);
    }

    void addEmp(Employee* addMe) {
        myGuys.push_back(addMe);
    }
};

int main() {

    // for timing
    auto begin = std::chrono::high_resolution_clock::now();

    std::vector<Supervisor*> supers;

    // put a lot of copies of it into a vector
    for (int i = 0; true; i++) {

        supers.push_back(new Supervisor("Joe Johnson", 20 + (i % 29)));
        for (int j = 0; j < 10; j++) {
            Employee* temp = new Employee("Steve Stevens", 20 + ((i + j) % 29));
            supers[supers.size() - 1]->addEmp(temp);
        }

        if (i > 10371) {
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Duration to create all of the objects: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << " ns."
              << std::endl;

    for (auto v : supers)
        delete v;
}
