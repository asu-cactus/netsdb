#include "TcapIrTestsRunner.h"

#include "BuildTcapIrTests.h"

namespace pdb_tests {
void runBuildTcapIrTests(UnitTest& qunit) {
    buildTcapIrTest1(qunit);
    buildTcapIrTest2(qunit);
    buildTcapIrTest3(qunit);
    buildTcapIrTest4(qunit);
    buildTcapIrTest5(qunit);
    buildTcapIrTest6(qunit);
    buildTcapIrTest7(qunit);
}
}
