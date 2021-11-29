#ifndef ZB_COMPANY_H
#define ZB_COMPANY_H

#include "Object.h"
#include "PDBVector.h"
#include "Supervisor.h"
#include "Handle.h"

//  PRELOAD %ZB_Company%

namespace pdb {

class ZB_Company : public Object {

private:
    Vector<int> labels;
    Vector<Handle<Supervisor>> myGuys;

public:
    ENABLE_DEEP_COPY

    ~ZB_Company() {}
    ZB_Company() {}

    void addSupervisor(Handle<Supervisor> supToAdd, int labelToAdd) {
        labels.push_back(labelToAdd);
        myGuys.push_back(supToAdd);
    }

    Handle<Supervisor>& getSupervisor(int which) {
        return myGuys[which];
    }

    void print() {
        for (size_t i = 0; i < labels.size(); i++) {
            std::cout << labels[i] << " ";
            myGuys[i]->print();
        }
    }
};
}

#endif
