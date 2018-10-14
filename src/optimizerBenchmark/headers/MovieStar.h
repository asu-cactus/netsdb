#ifndef MovieStar_H
#define MovieStar_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

class MovieStar : public pdb::Object {

public:
    pdb::Handle<pdb::String> name;
    pdb::Handle<pdb::String> address;
    char gender;
    int birthYear;

    int checkBirthYear = 1960;

    ENABLE_DEEP_COPY

    // Default constructor and destructor:
    ~MovieStar() {}
    MovieStar() {}


    // Constructor with arguments using std::string

    MovieStar(std::string name, std::string address, char gender, int birthYear) {
        this->name = pdb::makeObject<pdb::String>(name);
        this->address = pdb::makeObject<pdb::String>(address);
        this->gender = gender;
        this->birthYear = birthYear;
    }

    const pdb::Handle<pdb::String>& getName() const {
        return name;
    }

    void setName(const pdb::Handle<pdb::String>& name) {
        this->name = name;
    }


    const pdb::Handle<pdb::String>& getAddress() const {
        return address;
    }

    void setAddress(const pdb::Handle<pdb::String>& address) {
        this->address = address;
    }
};


#endif
