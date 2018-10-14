#ifndef SIMPLE_MOVIE_JOIN_H
#define SIMPLE_MOVIE_JOIN_H

// by Sourav, Jun 2017

#include "JoinComp.h"
#include "LambdaCreationFunctions.h"


#include "MovieStar.h"
#include "StarsIn.h"


using namespace pdb;

class SimpleMovieJoin : public JoinComp<MovieStar, MovieStar, StarsIn> {

public:
    ENABLE_DEEP_COPY

    SimpleMovieJoin() {}

    Lambda<bool> getSelection(Handle<MovieStar> in1, Handle<StarsIn> in2) override {
        return (makeLambdaFromMember(in1, name) == makeLambdaFromMember(in2, starName));
    }

    Lambda<Handle<MovieStar>> getProjection(Handle<MovieStar> in1, Handle<StarsIn> in2) override {
        return makeLambda(in1, [](Handle<MovieStar>& in1) { return in1; });
    }
};

#endif
