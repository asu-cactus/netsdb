#ifndef LINK_PARTITION_H
#define LINK_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "PartitionComp.h"
#include "Link.h"

using namespace pdb;

class LinkPartition : public PartitionComp<int, Link> {

public:
   ENABLE_DEEP_COPY

   LinkPartition() {}

   LinkPartition(std::string dbName, std::string setName) {
       this->setOutput(dbName, setName);
   }

   Lambda<int> getProjection(Handle<Link> page) override {
       return makeLambda(page, [this](Handle<Link> &page) {
            return page->url; 
        });

   }

};


#endif
