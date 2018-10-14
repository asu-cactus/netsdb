
#ifndef LDA_DOCUMENT_H
#define LDA_DOCUMENT_H

#include "Object.h"
#include "Handle.h"

/* The class for LDA documents */
using namespace pdb;

class LDADocument : public Object {

/* Document ID, word ID, and the number of this word in this document */
private:
    unsigned docID;
    unsigned wordID;
    unsigned count;

public:
    ENABLE_DEEP_COPY

    LDADocument() {}

    void setDoc(unsigned fromDoc) {
        this->docID = fromDoc;
    }

    unsigned getDoc() {
        return this->docID;
    }

    void setWord(int fromWord) {
        this->wordID = fromWord;
    }

    unsigned getWord() {
        return this->wordID;
    }

    void setCount(unsigned fromCount) {
        this->count = fromCount;
    }

    unsigned getCount() {
        return this->count;
    }

    void print() {
        std::cout << "docID: " << this->docID << " wordID: " << wordID << " count: " << count
                  << std::endl;
    }

    ~LDADocument() {}
};

#endif
