#ifndef REDDIT_AUTHOR_H
#define REDDIT_AUTHOR_H

#include <iostream>
#include <vector>
#include <sstream>
#include <stdlib.h>

//this file describes a reddit author


using namespace pdb;

namespace reddit {

class Author : public Object {

public:

    ENABLE_DEEP_COPY

    int author_id;
    String author = "";
    long field02;
    long field03;
    int field04;
    int field05;


    //default constructor
    Author () {}

    //constructor from a CSV string
    Author ( std::string csvString ) {
         std::stringstream s_stream(csvString);
         std::vector<std::string> result;
         while(s_stream.good()) {
             std::string substr;
             getline(s_stream, substr, ' ');
             result.push_back(substr);
         }
         author_id = atoi(result[0].c_str());
         author = result[1];
         field02 = atol(result[2].c_str());
         field03 = atol(result[3].c_str());
         field04 = atoi(result[4].c_str());
         field05 = atoi(result[5].c_str());
           

    }

    size_t hash() const override{
        return author.hash();
    }
};


}


#endif
