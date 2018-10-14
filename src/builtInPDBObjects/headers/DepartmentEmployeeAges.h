
#ifndef DEPARTMENT_EMPLOYEE_AGES_H
#define DEPARTMENT_EMPLOYEE_AGES_H

#include "Object.h"
#include "PDBString.h"
#include "Employee.h"
#include "PDBMap.h"
#include "PDBVector.h"

//  PRELOAD %DepartmentEmployeeAges%

namespace pdb {

class DepartmentEmployeeAges : public Object {

public:
    DepartmentEmployeeAges() {
        employees = makeObject<Map<String, Vector<int>>>();
    }

    ~DepartmentEmployeeAges() {}

    String departmentName;  // the name of the department
    Handle<Map<String, Vector<int>>>
        employees;  // for each supervisor in this department, the list of employees
    ENABLE_DEEP_COPY


    String& getKey() {
        return departmentName;
    }

    Handle<Map<String, Vector<int>>>& getValue() {
        return employees;
    }

    void print() {
        std::cout << "Department: " << departmentName << std::endl;
        auto iter = employees->begin();
        while (iter != employees->end()) {
            std::cout << "----Supervisor: " << (*iter).key << std::endl;
            std::cout << "----NumEmployees: " << (*iter).value.size() << std::endl;
            for (int i = 0; i < (*iter).value.size(); i++) {
                std::cout << i << ": " << (((*iter).value)[i]) << ";";
            }
            std::cout << std::endl;
            ++iter;
        }
    }
};

inline Handle<Map<String, Vector<int>>>& operator+(Handle<Map<String, Vector<int>>>& lhs,
                                                   Handle<Map<String, Vector<int>>>& rhs) {
    auto iter = rhs->begin();
    while (iter != rhs->end()) {
        String myKey = (*iter).key;
        if (lhs->count(myKey) == 0) {
            try {
                (*lhs)[myKey] = (*iter).value;
            } catch (NotEnoughSpace& n) {
                // std :: cout << "not enough space when inserting new pair" << std :: endl;
                lhs->setUnused(myKey);
                throw n;
            }
        } else {

            size_t mySize = (*lhs)[myKey].size();
            size_t otherSize = (*iter).value.size();
            // std :: cout << "mySize is " << mySize << " and otherSize is " << otherSize << std ::
            // endl;
            for (size_t i = mySize; i < mySize + otherSize; i++) {
                try {

                    (*lhs)[myKey].push_back((*iter).value[i - mySize]);

                } catch (NotEnoughSpace& n) {

                    // std :: cout << i << ": not enough space when updating value for pushing back:
                    // " << (*lhs)[myKey].size() << std :: endl;
                    size_t curSize = (*lhs)[myKey].size();
                    for (size_t j = mySize; j < curSize; j++) {
                        (*lhs)[myKey].pop_back();
                    }
                    // std :: cout << "size restored to " << (*lhs)[myKey].size() << std :: endl;
                    for (size_t j = 0; j < (*lhs)[myKey].size(); j++) {
                        std::cout << j << ": " << (*lhs)[myKey][j] << ";";
                    }
                    throw n;
                }
            }
            // std :: cout << "now my size is " << (*lhs)[myKey].size() << std :: endl;
        }
        ++iter;
    }
    return lhs;
}
}

#endif
