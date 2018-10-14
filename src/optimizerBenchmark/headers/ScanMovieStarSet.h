#ifndef SCAN_MOVIESTAR_SET_H
#define SCAN_MOVIESTAR_SET_H

#include "ScanUserSet.h"
#include "MovieStar.h"

using namespace pdb;
class ScanMovieStarSet : public ScanUserSet<MovieStar> {

public:
    ENABLE_DEEP_COPY

    ScanMovieStarSet() {}

    ScanMovieStarSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};

#endif