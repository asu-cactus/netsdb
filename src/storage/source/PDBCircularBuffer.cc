
#ifndef PDB_CIRCULAR_BUFFER_CC
#define PDB_CIRCULAR_BUFFER_CC

#include "PDBCircularBuffer.h"
#include <string>
#include <iostream>
#include <stdexcept>
#include <cstdlib>


template <class T>
PDBCircularBuffer<T>::PDBCircularBuffer(unsigned int bufferSize, pdb::PDBLoggerPtr logger) {
    this->maxArraySize = bufferSize + 1;
    this->logger = logger;
    this->initArray();
}

template <class T>
PDBCircularBuffer<T>::~PDBCircularBuffer() {
    // the buffer is not responsible for freeing the elements in the buffer
    delete[] this->array;
}

template <class T>
int PDBCircularBuffer<T>::initArray() {
    this->array = new T[this->maxArraySize];
    if (this->array == nullptr) {
        std::cout << "PDBCircularBuffer: Out of Memory in Heap.\n";
        this->logger->writeLn("PDBCircularBuffer: Out of Memory in Heap.");
        return -1;
    }
    int i;
    for (i = 0; i < this->maxArraySize; i++) {

        this->array[i] = nullptr;
    }
    this->arrayHead = 0;
    this->arrayTail = 0;
    return 0;
}

template <class T>
int PDBCircularBuffer<T>::addToTail(T const& elem) {
    if (this->isFull()) {
        return -1;
    }
    this->logger->writeLn(
        "PDBCircularBuffer<T>: the buffer is not full, adding the element to tail...");
    this->arrayTail = (this->arrayTail + 1) % this->maxArraySize;
    this->array[this->arrayTail] = elem;

    return 0;
}

template <class T>
T PDBCircularBuffer<T>::popFromHead() {
    if (this->isEmpty()) {
        this->logger->writeLn("PDBCircularBuffer: array is empty.");
        throw std::out_of_range("PDBCircularBuffer<>::popFromHead(): empty buffer");
    }
    this->arrayHead = (this->arrayHead + 1) % this->maxArraySize;
    T elem = this->array[this->arrayHead];
    return elem;
}

template <class T>
bool PDBCircularBuffer<T>::isFull() {
    return (this->arrayHead == (this->arrayTail + 1) % this->maxArraySize);
}

template <class T>
bool PDBCircularBuffer<T>::isEmpty() {
    return (this->arrayHead == this->arrayTail);
}

template <class T>
unsigned int PDBCircularBuffer<T>::getSize() {
    return (this->arrayTail - this->arrayHead + this->maxArraySize) % this->maxArraySize;
}

#endif
