#include <iostream>
#include <cassert>

#include "LALexer.h"
#include "LAParser.h"
#include "LAPDBInstance.h"
#include "LAStatementsList.h"

// by Binhang, June 2017


int main(int argc, char** argv) {

    assert(argc == 6);
    bool printResult = (strcmp(argv[1], "Y") == 0);
    bool clusterMode = (strcmp(argv[2], "Y") == 0);
    size_t blockSize = atoi(argv[3]);
    int port = 8108;
    std::string masterIP = argv[4];
    std::string jobName = argv[5];
    FILE* targetCode = fopen(argv[5], "r");

    if (!targetCode) {
        std::cout << "No such file ! <" << argv[5] << ">" << std::endl;
        return -1;
    }

    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("LAclientLog");
    LAPDBInstance instance(printResult, clusterMode, blockSize, masterIP, port, clientLogger, jobName);

    LAscan_t myscanner;
    LAlex_init(&myscanner);
    LAset_in(targetCode, myscanner);
    std::cout << "Get started to parse the code!" << std::endl;
    LAStatementsList* myStatements = new LAStatementsList();
    LAparse(myscanner, &myStatements);
    LAlex_destroy(myscanner);
    std::cout << "Parsing Done:" << std::endl;
    for (int i = 0; i < myStatements->size(); i++) {
        std::cout << myStatements->get(i)->toString() << std::endl;
    }
    std::cout << "Start executation:" << std::endl;
    for (int i = 0; i < myStatements->size(); i++) {
        std::cout << "Current statement:" << myStatements->get(i)->toString() << std::endl;
        myStatements->get(i)->evaluateQuery(instance);
    }
    instance.clearCachedSets();
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std::cout << "Can't cleanup so files" << std::endl;
    }
}
