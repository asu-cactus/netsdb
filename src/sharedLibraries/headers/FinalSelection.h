
#ifndef FINAL_SELECTION_H
#define FINAL_SELECTION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "DepartmentTotal.h"
#include "PDBVector.h"
#include "PDBString.h"


using namespace pdb;
class FinalSelection : public SelectionComp<double, DepartmentTotal> {

public:
    ENABLE_DEEP_COPY

    FinalSelection() {}

    Lambda<bool> getSelection(Handle<DepartmentTotal> checkMe) override {
        return makeLambdaFromMethod(checkMe, checkSales);
    }

    Lambda<Handle<double>> getProjection(Handle<DepartmentTotal> checkMe) override {
        return makeLambdaFromMethod(checkMe, getTotSales);
    }
};


#endif
