
#ifndef PAGECIRCULARBUFFER_H
#define PAGECIRCULARBUFFER_H

#include "PDBPage.h"
#include "PDBLogger.h"
#include <pthread.h>
#include <memory>
using namespace std;
class PageCircularBuffer;
typedef shared_ptr<PageCircularBuffer> PageCircularBufferPtr;

/**
 * This class implements a concurrent blocking circular buffer for producer-consumer problems.
 * The consumer threads will wait until there are pages available in the buffer.
 * The producer threads will wait until there are rooms available in the buffer to push back new
 * pages.
 */

class PageCircularBuffer {
public:
    PageCircularBuffer(unsigned int bufferSize, pdb::PDBLoggerPtr logger);
    ~PageCircularBuffer();

    /**
     * Add page to the tail of the circular buffer.
     * If the buffer is full, it will block until there is new room in the buffer.
     */
    int addPageToTail(PDBPagePtr page);

    /**
     * Pop page from the head of the circular buffer.
     * If the buffer is empty, it will block until there is new page added to in the buffer,
     * or until the buffer is closed.
     * If the buffer is empty while being closed, the function will return nullptr.
     */
    PDBPagePtr popPageFromHead();

    /**
     * If the buffer is full, return true, otherwise, return false.
     */
    bool isFull();

    /**
     * If the buffer is empty, return true, otherwise, return false.
     */
    bool isEmpty();

    /**
     * Return the current size of the circular buffer.
     */
    unsigned int getSize();

    /**
     * Close the buffer.
     * If the buffer is empty now, notify all consumer threads that the buffer is closed.
     */
    void close();

    /**
      * Open the buffer.
      */
    void open();


    /**
     * If the buffer is closed, return true, otherwise, return false.
     */
    bool isClosed() {
        return closed;
    }

protected:
    /**
     * Return the page array used to construct the concurrent blocking circular buffer.
     */
    PDBPagePtr* getPageArray() {
        return pageArray;
    }

    /**
     * Return the head index to the concurrent blocking circular buffer.
     */
    unsigned int getPageArrayHead() {
        return pageArrayHead;
    }

    /**
     * Return the tail index to the concurrent blocking circular buffer.
     */
    unsigned int getPageArrayTail() {
        return pageArrayTail;
    }

    /**
     * Return the maximum size of the concurrent blocking circular buffer.
     */
    unsigned int getMaxArraySize() {
        return maxArraySize;
    }

    /**
     * Initialize the concurrent blocking circular buffer.
     */
    int initArray();

private:
    PDBPagePtr* pageArray;
    pdb::PDBLoggerPtr logger;
    unsigned int maxArraySize;
    unsigned int pageArrayHead;
    unsigned int pageArrayTail;
    pthread_mutex_t mutex;
    pthread_mutex_t addPageMutex;
    pthread_cond_t cond;
    bool closed;
};


#endif /* PAGECIRCULARBUFFER_H */
