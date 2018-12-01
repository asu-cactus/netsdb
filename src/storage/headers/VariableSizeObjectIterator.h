#ifndef VARIABLE_SIZE_OBJECT_ITERATOR_H
#define VARIABLE_SIZE_OBJECT_ITERATOR_H


#include "PDBPage.h"
#include "Record.h"
#include <memory>


class VariableSizeObjectIterator;
typedef std::shared_ptr<VariableSizeObjectIterator> VariableSizeObjectIteratorPtr;


using namespace pdb;
/**
 * This class wraps an iterator to scan records in a page.
 */
class VariableSizeObjectIterator {

public:
    VariableSizeObjectIterator(PDBPagePtr page);
    ~VariableSizeObjectIterator();

    /**
     * Returns a pointer to a Record.
     * Returns nullptr, if there is no more record in the page.
     */
    char * next(size_t& size);

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
