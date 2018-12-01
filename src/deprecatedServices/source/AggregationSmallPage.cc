/*
 * File: AggregationSmallPage.cc
 * Author: Jia
 *
 * Created on Apr 9, 2016, 13:27 PM
 */

#ifndef AGGREGATIONSMALLPAGE_CC
#define AGGREGATIONSMALLPAGE_CC

#include "AggregationSmallPage.h"
#include <string.h>
AggregationSmallPage::AggregationSmallPage(char * bytes, size_t size, void * dest) {
    this->bytes = dest;
    this->size = size;
    *((int *) dest) = 0;
    memcpy((char *)dest+sizeof(int), bytes, size);
}

AggregationSmallPage::~AggregationSmallPage() {
    
}

void AggregationSmallPage::updateObject(int count) {
    *((int *) bytes) = *((int *) bytes) + count;
}

#endif
