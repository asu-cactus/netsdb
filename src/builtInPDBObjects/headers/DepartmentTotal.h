
#ifndef DEPARTMENTAL_TOTAL_H
#define DEPARTMENTAL_TOTAL_H

#include "Object.h"
#include "PDBString.h"

//  PRELOAD %DepartmentTotal%

namespace pdb {

class DepartmentTotal : public Object {

public:
    double totSales;
    String departmentName;

    ENABLE_DEEP_COPY

    bool checkSales() {
        if ((((int)(totSales * 10)) + 5) / 10 == ((int)(totSales * 10)) / 10) {
            return true;
        }
        return false;
    }

    Handle<double> getTotSales() {
        Handle<double> returnVal = makeObject<double>(totSales);
        return returnVal;
    }

    String& getKey() {
        return departmentName;
    }

    double& getValue() {
        return totSales;
    }

    void print() {
        std::cout << departmentName << ":" << totSales << std::endl;
    }
};
}

#endif
