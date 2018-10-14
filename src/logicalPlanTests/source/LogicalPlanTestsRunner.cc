#include "LogicalPlanTestsRunner.h"

#include "BuildLogicalPlanTests.h"

namespace pdb_tests {
void runLogicalPlanTests(UnitTest& qunit) {
    testBuildLogicalPlanFromLoad(qunit);
    testBuildLogicalPlanFromApplyFunction(qunit);
    testBuildLogicalPlanFromApplyMethod(qunit);
    testBuildLogicalPlanFromHoist(qunit);
    testBuildLogicalPlanFromGreaterThan(qunit);
    testBuildLogicalPlanFromFilter(qunit);
    testBuildLogicalPlanFromStore(qunit);
    testBuildLogicalPlanFromStringTest47(qunit);
}
}
