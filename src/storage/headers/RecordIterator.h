#ifndef RECORD_ITERATOR_H
#define RECORD_ITERATOR_H


#include "PDBPage.h"
#include "Record.h"
#include "Object.h"
#include <memory>


class RecordIterator;
typedef std::shared_ptr<RecordIterator> RecordIteratorPtr;


using namespace pdb;
/**
 * This class wraps an iterator to scan records in a page.
 */
class RecordIterator {

public:
    RecordIterator(PDBPagePtr page);
    ~RecordIterator();

    /**
     * Returns a pointer to a Record.
     * Returns nullptr, if there is no more record in the page.
     */
    Record<Object>* next();

    /**
     * Returns true, if there is more objects in the page, otherwise returns false.
     */
    bool hasNext();

private:
    PDBPagePtr page;
    int numObjectsInCurPage;
    char* curPosInPage;
    int numObjectsIterated;
};


#endif
