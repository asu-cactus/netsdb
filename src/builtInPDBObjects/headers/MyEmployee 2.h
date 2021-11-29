
#ifndef MYEMPLOYEE_H
#define MYEMPLOYEE_H

#include "Object.h"
#include "Handle.h"

//  PRELOAD %MyEmployee%

namespace pdb {


class MyEmployee : public Object {

private:
    int age;
    int salary;

public:
    ENABLE_DEEP_COPY

    ~MyEmployee() {}

    MyEmployee() {}

    MyEmployee(int ageIn, int salaryIn) {
        age = ageIn;
        salary = salaryIn;
    }

    int getAge() {
        return age;
    }

    int getSalary() {
        return salary;
    }

    void print() {
        std::cout << "age: " << age << "; salary: " << salary << std::endl;
    }
};
}

#endif
