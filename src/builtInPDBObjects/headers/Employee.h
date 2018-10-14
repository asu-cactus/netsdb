
#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"

//  PRELOAD %Employee%

namespace pdb {

class Employee : public Object {

    Handle<String> name;
    int age;

public:
    double salary;
    String department;

    ENABLE_DEEP_COPY

    ~Employee() {}
    Employee() {}
    size_t hash() {
        return name->hash();
    }
    void print() {
        std::cout << "name is: " << *name << " age is: " << age << " dept is: " << department;
    }

    Handle<String>& getName() {
        return name;
    }

    bool isFrank() {
        return (*name == "Frank");
    }

    int getAge() {
        return age;
    }

    double getSalary() {
        return salary;
    }

    Employee(std::string nameIn, int ageIn, std::string department, double salary)
        : salary(salary), department(department) {
        name = makeObject<String>(nameIn);
        age = ageIn;
    }

    Employee(std::string nameIn, int ageIn) {
        name = makeObject<String>(nameIn);
        age = ageIn;
        department = "myDept";
        salary = 123.45;
    }

    bool operator==(Employee& me) const {
        return name == me.name;
    }

    size_t hash () const {

        return name->hash();

    }

};
}

#endif
