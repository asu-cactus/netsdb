/*
 * File:   PDBCircularBuffer.h
 * Author: Jia
 *
 * Created on November 17, 2015, 11:27 AM
 */

#ifndef PDBCIRCULARBUFFER_H
#define PDBCIRCULARBUFFER_H


#include "PDBLogger.h"

template <class T>
class PDBCircularBuffer {
public:
    PDBCircularBuffer(unsigned int bufferSize, pdb::PDBLoggerPtr logger);
    ~PDBCircularBuffer();

    int addToTail(T const&);
    T popFromHead();
    bool isFull();
    bool isEmpty();
    unsigned int getSize();

protected:
    int initArray();

private:
    T* array;
    pdb::PDBLoggerPtr logger;
    unsigned int maxArraySize;
    unsigned int arrayHead;
    unsigned int arrayTail;
};


#endif /* PDBCIRCULARBUFFER_H */
