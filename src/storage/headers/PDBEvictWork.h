
#ifndef PDBEVICTWORK_H
#define PDBEVICTWORK_H

#include <memory>
using namespace std;
class PDBEvictWork;
typedef shared_ptr<PDBEvictWork> PDBEvictWorkPtr;

#include "PDBWork.h"
#include "PDBBuzzer.h"
#include "PageCache.h"

/**
 * This class implements some work to do for cache eviction.
 */
class PDBEvictWork : public pdb::PDBWork {
public:
    PDBEvictWork(PageCache* cache);
    ~PDBEvictWork();

    // do the actual work.
    void execute(PDBBuzzerPtr callerBuzzer) override;

private:
    PageCache* cache;
};
#endif /* PDBEVICTWORK_H */
