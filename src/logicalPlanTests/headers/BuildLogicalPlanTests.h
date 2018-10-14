#ifndef PDB_LOGICALPLANTESTS_BUILDLOGICALPLANTESTS_H
#define PDB_LOGICALPLANTESTS_BUILDLOGICALPLANTESTS_H

#include "qunit.h"

using QUnit::UnitTest;

namespace pdb_tests {
void testBuildLogicalPlanFromLoad(UnitTest& qunit);

void testBuildLogicalPlanFromApplyFunction(UnitTest& qunit);

void testBuildLogicalPlanFromApplyMethod(UnitTest& qunit);

void testBuildLogicalPlanFromHoist(UnitTest& qunit);

void testBuildLogicalPlanFromGreaterThan(UnitTest& qunit);

void testBuildLogicalPlanFromFilter(UnitTest& qunit);

void testBuildLogicalPlanFromStore(UnitTest& qunit);

void testBuildLogicalPlanFromStringTest47(UnitTest& qunit);
}

#endif  // PDB_LOGICALPLANTESTS_BUILDLOGICALPLANTESTS_H
