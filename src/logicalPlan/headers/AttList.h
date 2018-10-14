
#ifndef ATT_LIST_H
#define ATT_LIST_H

#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>
#include <map>

// this is a list of attributes... used to build up schema specifications
struct AttList {

private:
    std::vector<std::string> atts;

public:
    ~AttList() {}
    AttList() {}

    void appendAttribute(char* appendMe) {
        atts.push_back(std::string(appendMe));
    }

    std::vector<std::string>& getAtts() {
        return atts;
    }

    friend struct TupleSpec;
};

#endif
