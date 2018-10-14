
#include <iostream>

#include "LogicalPlanTestsRunner.h"
#include "InterfaceFunctions.h"
#include "QueryItermediaryRepTestsRunner.h"
#include "QueriesTestsRunner.h"
#include "TcapBuilderTestsRunner.h"
#include "TcapTestsRunner.h"
#include "TcapParsersTestsRunner.h"
#include "TcapIrTestsRunner.h"
#include "qunit.h"


using QUnit::UnitTest;

using pdb::makeObjectAllocatorBlock;

using pdb_tests::runQueriesTests;
using pdb_tests::runQueryIrTests;
using pdb_tests::runTcapTests;
using pdb_tests::runTcapParserTests;
using pdb_tests::runBuildTcapIrTests;
using pdb_tests::runLogicalPlanTests;
using pdb_tests::runBuildTcapBuilderTests;

int main() {
    makeObjectAllocatorBlock(1024 * 10, true);

    UnitTest qunit(std::cerr, QUnit::normal);

    runQueriesTests(qunit);
    runQueryIrTests(qunit);
    runTcapTests(qunit);
    runTcapParserTests(qunit);
    runBuildTcapIrTests(qunit);
    runBuildTcapBuilderTests(qunit);
    runLogicalPlanTests(qunit);

    return qunit.errors();
}
