
#ifndef SIMPLE_EMPLOYEE_H
#define SIMPLE_EMPLOYEE_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"
#include "ExportableObject.h"
#include <vector>

class SimpleEmployee : public ExportableObject {

public:
    pdb::String name;
    int age;
    double salary;
    pdb::String department;

    ENABLE_DEEP_COPY

    ~SimpleEmployee() {}
    SimpleEmployee() {}

    void print() override {
        std::cout << name << "," << age << "," << salary << "," << department << std::endl;
    }


    std::string toSchemaString(std::string format) override {
        if (format == "csv") {
            return "name,age,salary,department\n";
        } else {
            return "";
        }
    }

    std::string toValueString(std::string format) override {
        if (format == "csv") {
            char buffer[65535];
            sprintf(buffer, "%s,%d,%f,%s\n", name.c_str(), age, salary, department.c_str());
            return buffer;
        } else {
            return "";
        }
    }


    std::vector<std::string> getSupportedFormats() override {
        std::vector<std::string> ret;
        ret.push_back("csv");
        return ret;
    }


    SimpleEmployee(std::string nameIn, int ageIn, double salaryIn, std::string departmentIn) {
        name = nameIn;
        age = ageIn;
        salary = salaryIn;
        department = departmentIn;
    }
};


#endif
