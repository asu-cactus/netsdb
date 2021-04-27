#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_PAGEINDEXER_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_PAGEINDEXER_H_

#include "AbstractIndexer.h"
#include "Handle.h"
#include "Object.h"
#include "PDBPage.h"
#include <memory>

#include "DataTypes.h"
#include <iostream>
#include <unordered_map>
#include <vector>

class PageIndexer;
typedef std::shared_ptr<PageIndexer> PageIndexerPtr;

//  PRELOAD %PageIndexer <Nothing, Nothing>%

namespace pdb {

template <class KeyType, class ValueType>
class PageIndexer : public AbstractIndexer {

  std::unordered_map<KeyType, std::vector<SharedPageID>> *pageMap;

public:

  PageIndexer() {}
  ~PageIndexer() {}

  void initialize() override {
    pageMap = new std::unordered_map<KeyType, std::vector<SharedPageID>>();
  }

  void addPage(PDBPagePtr myPage) override {
    SharedPageID pid;
    pid.dbId = myPage->getDbID();
    pid.nodeId = myPage->getNodeID();
    pid.pageId = myPage->getPageID();
    pid.setId = myPage->getSetID();

    Record<Vector<Handle<ValueType>>> *myRec1 =
        (Record<Vector<Handle<ValueType>>> *)(myPage->getBytes());
    Handle<Vector<Handle<ValueType>>> inputVec1 = myRec1->getRootObject();

    KeyType key = hashSet(*inputVec1);
    if (pageMap->count(key) == 0) {
      pageMap->emplace(key, std::vector<SharedPageID>());
    }
    pageMap->at(key).push_back(pid);
  }

  virtual SharedPageID *checkAndAddPage(PDBPagePtr page) override {
    Record<Vector<Handle<ValueType>>> *myRec1 =
        (Record<Vector<Handle<ValueType>>> *)(page->getBytes());
    Handle<Vector<Handle<ValueType>>> inputVec1 = myRec1->getRootObject();

    KeyType key = hashSet(*inputVec1);
    std::cout << "[PAGEINDEXER] GENERATED KEY: " << key << std::endl;

    if (pageMap->count(key) == 0) {
      SharedPageID pid;
      pid.dbId = page->getDbID();
      pid.nodeId = page->getNodeID();
      pid.pageId = page->getPageID();
      pid.setId = page->getSetID();

      if (pageMap->count(key) == 0) {
        pageMap->emplace(key, std::vector<SharedPageID>());
      }
      pageMap->at(key).push_back(pid);

      return nullptr;
    }

    return &(pageMap->at(key)[0]);
  }


  SharedPageID *queryPage(PDBPagePtr myPage) override {
    Record<Vector<Handle<ValueType>>> *myRec1 =
        (Record<Vector<Handle<ValueType>>> *)(myPage->getBytes());
    Handle<Vector<Handle<ValueType>>> inputVec1 = myRec1->getRootObject();

    KeyType key = hashSet(*inputVec1);
    std::cout << "[PAGEINDEXER] GENERATED KEY: " << key << std::endl;

    if (pageMap->count(key) == 0)
      return nullptr;
    return &(pageMap->at(key)[0]);
  }

  virtual KeyType hashSet(Vector<Handle<ValueType>> &objects) = 0;

  virtual void dump() override {
    std::cout << "Called PageIndexer dump: " << std::endl;
  }
};
} // namespace pdb

#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_PAGEINDEXER_H_ */
