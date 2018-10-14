#ifndef StarsIn_H
#define StarsIn_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

class StarsIn : public pdb::Object {

public:
    pdb::Handle<pdb::String> movieTitle;
    pdb::Handle<pdb::String> starName;
    int movieYear;

    ENABLE_DEEP_COPY

    // Default constructor and destructor:
    ~StarsIn() {}
    StarsIn() {}


    // Constructor with arguments using std::string
    StarsIn(std::string movieTitle, std::string starName, int movieYear) {
        this->movieTitle = pdb::makeObject<pdb::String>(movieTitle);
        this->starName = pdb::makeObject<pdb::String>(starName);
        this->movieYear = movieYear;
    }
};

#endif