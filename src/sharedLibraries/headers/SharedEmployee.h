
#ifndef SHARED_EMPLOYEE_H
#define SHARED_EMPLOYEE_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"
#include "ExportableObject.h"
#include <vector>

class SharedEmployee : public ExportableObject {

    int age;
    double salary;

public:
    pdb::Handle<pdb::String> name;

    ENABLE_DEEP_COPY

    ~SharedEmployee() {}
    SharedEmployee() {}

    void print() override {
        std::cout << "name is: " << *name << " age is: " << age;
    }

    std::string toSchemaString(std::string format) override {
        if (format == "csv") {
            return "name,age,salary\n";
        } else {
            return "";
        }
    }

    std::string toValueString(std::string format) override {
        if (format == "csv") {
            char buffer[65535];
            sprintf(buffer, "%s,%d,%f\n", name->c_str(), age, salary);
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


    pdb::Handle<pdb::String>& getName() {
        return name;
    }

    bool isFrank() {
        return (*name == "Frank");
    }

    SharedEmployee(std::string nameIn, int ageIn) {
        name = pdb::makeObject<pdb::String>(nameIn);
        age = ageIn;
        salary = 4000.0;
    }

    SharedEmployee(std::string nameIn, int ageIn, double salaryIn) {
        name = pdb::makeObject<pdb::String>(nameIn);
        age = ageIn;
        salary = salaryIn;
    }


    double getSalary() {
        return salary;
    }
};


#endif
