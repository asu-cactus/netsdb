/*
 * File: SmallPage.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 14:02 PM
 */

#ifndef SMALLPAGE_H
#define SMALLPAGE_H

#include "DataTypes.h"
#include "PDBObject.h"
#include <string.h>
#include <iostream>
#include <memory>
using namespace std;
class SmallPage;
typedef shared_ptr<SmallPage> SmallPagePtr;

/*
 * The SmallPage class wraps a piece of memory that is allocated from a large page. It provides method for user to add objects to the small page. For reading, user will read the large page using fixed size or variable size object iterator directly. 
 **/
class SmallPage {
public:
    SmallPage(void * bytes, size_t size, SmallPageType type);
    ~SmallPage();
    bool addObject(PDBObjectPtr object);
    inline bool addObjectByBytes(void * bytes, size_t size) {
        if(this->type != FixedSize) {
            if(remainingSize < size ) {
                return false;
            }
            memcpy ((char *)(this->bytes)+this->curOffsetInSmallPage, (char *)bytes, size);
            return true;
        }

        if(remainingSize < (size + sizeof(size_t))) {
            *((int *) this->bytes) = this->numObjects;
            return false;
        }
        *((short *) ((char *)(this->bytes)+this->curOffsetInSmallPage)) = (short)size;
        this->curOffsetInSmallPage += sizeof(short);
        remainingSize -= sizeof(short);
        memcpy ((char *)(this->bytes)+this->curOffsetInSmallPage, (char *)bytes, size);
        this->curOffsetInSmallPage += size;
        remainingSize -= size;
        this->numObjects ++;
        return true;
}
    size_t getSize() {
       return this->size;
    }

private:
   void * bytes;
   size_t size;
   size_t curOffsetInSmallPage;
   SmallPageType type;
   int numObjects;
   size_t remainingSize;
};



#endif
