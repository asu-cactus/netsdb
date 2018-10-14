#include <iostream>

#include "LAParser.h"
#include "LAStatementsList.h"

// by Binhang, June 2017


int main(int argc, char** argv) {

    if (argc == 2) {
        FILE* targetCode = fopen(argv[1], "r");
        if (!targetCode) {
            std::cout << "No such file ! <" << argv[1] << ">" << std::endl;
            return -1;
        }

        LAscan_t myscanner;

        LAlex_init(&myscanner);

        LAset_in(targetCode, myscanner);

        std::cout << "Get started to parse the file!" << std::endl;

        LAStatementsList* myStatements = new LAStatementsList();

        LAparse(myscanner, &myStatements);

        LAlex_destroy(myscanner);

        std::cout << "Parsing Done" << std::endl;

        for (int i = 0; i < myStatements->size(); i++) {
            std::cout << myStatements->get(i)->toString() << std::endl;
        }
    }
}
