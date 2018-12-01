/*
 * File: AggregationSmallPage.h
 * Author: Jia
 *
 * Created on Apr 9, 2016, 13:04 PM
 */

#ifndef AGGREGATIONSMALLPAGE_H
#define AGGREGATIONSMALLPAGE_H

#include <memory>
using namespace std;
class AggregationSmallPage;
typedef shared_ptr<AggregationSmallPage> AggregationSmallPagePtr;

/*
 * The AggregationSmallPage class wraps a special SmallPage class for aggregation. So the first 4 bytes represent the int value as count, the second 4 bytes represent the key string length K, and the following bytes of length K represent the string key.
 **/

class AggregationSmallPage {
public:
    AggregationSmallPage(char * bytes, size_t size, void * dest);
    ~AggregationSmallPage();
    void updateObject (int count); 
    void setUnpinned () { this->bytes = nullptr; }
    bool isUnpinned() { 
       if(this->bytes == nullptr) { 
           return true; 
       } else { 
           return false;
       } 
    }
private:
    void * bytes;
    size_t size;
};




#endif
