#ifndef SIMPLE_MOVIE_SELECTION
#define SIMPLE_MOVIE_SELECTION

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "PDBVector.h"
#include "PDBString.h"

#include "MovieStar.h"
#include "StarsIn.h"


using namespace pdb;
class SimpleMovieSelection : public SelectionComp<MovieStar, MovieStar> {

public:
    ENABLE_DEEP_COPY

    SimpleMovieSelection() {}

    Lambda<bool> getSelection(Handle<MovieStar> checkMe) override {
        return makeLambdaFromMember(checkMe, birthYear) ==
            makeLambdaFromMember(checkMe, checkBirthYear);
    }

    Lambda<Handle<MovieStar>> getProjection(Handle<MovieStar> checkMe) override {
        return makeLambda(checkMe, [](Handle<MovieStar>& checkMe) { return checkMe; });
    }
};


#endif
