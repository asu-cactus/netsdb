#ifndef EmbeddingSegment_H
#define EmbeddingSegment_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"
#include <vector>

using namespace pdb;

class EmbeddingSegment : public Object {

public:
    int id;
    int startColIndex;
    int length;
    Handle<Vector<double>> embedding;

    ENABLE_DEEP_COPY

    ~EmbeddingSegment() {}
    EmbeddingSegment() {}

    EmbeddingSegment(int id, int startColIndex, int length, Handle<Vector<double>> embedding) {
        this->id = id;
        this->startColIndex = startColIndex;
        this->length = length;
        this->embedding = makeObject<Vector<double>>();
        this->embedding = embedding;
    }

};


#endif
