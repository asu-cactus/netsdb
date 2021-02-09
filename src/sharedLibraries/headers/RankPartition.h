#ifndef RANK_PARTITION_H
#define RANK_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "PartitionComp.h"
#include "RankedUrl.h"

using namespace pdb;

class RankPartition : public PartitionComp<int, RankedUrl> {

public:
   ENABLE_DEEP_COPY

   RankPartition() {}

   RankPartition(std::string dbName, std::string setName) {
       this->setOutput(dbName, setName);
   }

   Lambda<int> getProjection(Handle<RankedUrl> page) override {
       return makeLambda(page, [this](Handle<RankedUrl> &page) {
            return page->url; 
        });

   }

};


#endif
