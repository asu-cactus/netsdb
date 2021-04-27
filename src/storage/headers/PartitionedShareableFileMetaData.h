#ifndef SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONEDSHAREABLEFILEMETADATA_H_
#define SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONEDSHAREABLEFILEMETADATA_H_

#include "DataTypes.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;
using namespace pdb;

#include "FFMatrixBlock.h"
#include "PDBVector.h"
#include "Handle.h"
#include "Record.h"

#include "UserSet.h"

class PartitionedShareableFileMetaData;
typedef shared_ptr<PartitionedShareableFileMetaData>
    PartitionedShareableFileMetaDataPtr;

class PartitionedShareableFileMetaData {
private:
  unsigned short version;
  vector<SharedPageID> *sharedPageIds;
  pthread_mutex_t sharedPageIdsMutex;

  string meta_root;
  string meta_path;
  string borrower_info_path;
  string borrowing_info_path;

  FILE *borrower_info_file = nullptr;
  FILE *borrowing_info_file = nullptr;
  pthread_mutex_t fileMutex;

  unsigned int total_shared_pages;
  bool upToDate;

  bool cleared;

  static string getBorrowerFile(string metaPath) {
    char buffer[500];
    sprintf(buffer, "%s/borrower", metaPath.c_str());
    return string(buffer);
  }

  static string getBorrowingFile(string metaPath) {
    char buffer[500];
    sprintf(buffer, "%s/borrowing", metaPath.c_str());
    return string(buffer);
  }

  static void createDir(string path) {
    struct stat st = {0};
    if (stat(path.c_str(), &st) == -1) {
      mkdir(path.c_str(), 0777);
    }
  }

public:
  static string getMetaPath(string metaPath) {
    char buffer[500];
    sprintf(buffer, "%s.shared", metaPath.c_str());
    return string(buffer);
  }

  static string getMetaPath(string typePath, SetID setId, string setName) {
    char buffer[500];
    sprintf(buffer, "%s/%d_%s.shared", typePath.c_str(), setId,
            setName.c_str());
    return string(buffer);
  }

  static string getMetaFile(string typePath, SetID setId, string setName) {
    char buffer[500];
    sprintf(buffer, "%s/%d_%s.shared/meta", typePath.c_str(), setId,
            setName.c_str());
    return string(buffer);
  }

  static string getMetaFile(string metaPath) {
    char buffer[500];
    sprintf(buffer, "%s/meta", metaPath.c_str());
    return string(buffer);
  }

  string getMetaFile() {
      return getMetaFile(meta_path);
  }

  PartitionedShareableFileMetaData(string metaPath)
      : meta_path(metaPath),
        borrower_info_path(getBorrowerFile(meta_path)),
        borrowing_info_path(getBorrowingFile(meta_path)) {
    meta_root = metaPath.substr(0, metaPath.rfind("/"));
    std::cout << "Parsed metaroot as " << meta_root << std::endl;
    version = 0;
    total_shared_pages = 0;
    sharedPageIds = new vector<SharedPageID>();
    pthread_mutex_init(&sharedPageIdsMutex, nullptr);
    pthread_mutex_init(&fileMutex, nullptr);
    cleared = false;
    upToDate = true;

    std::cout << "[PartitionedShareableFileMetaData] borrower metadata path: "
              << borrower_info_path
              << ", borrowing metadata path: " << borrowing_info_path
              << std::endl;
  }

  PartitionedShareableFileMetaData(string typeMetaPath, SetID setID,
                                   string setName)
      : PartitionedShareableFileMetaData(getMetaPath(typeMetaPath, setID, setName)) {
    meta_root = typeMetaPath;
    initialize();
  }

  bool initialize() {
    createDir(meta_path);
    if (!openMeta() || writeMeta() == 0)
      return false;
    return true;
  }

  bool closeMeta() {
    int res = fclose(borrowing_info_file);
    borrowing_info_file = nullptr;
    return res;
  }

  bool openMeta() {
    // check whether meta partition exists, if not, create it.
    if (borrowing_info_file == nullptr) {

      FILE *curFile;
      std::cout << "path to meta file is " << borrowing_info_path.c_str()
                << std::endl;
      ifstream file(borrowing_info_path.c_str());
      if (!file) {
        curFile = fopen(borrowing_info_path.c_str(), "w");
      } else {
        curFile = fopen(borrowing_info_path.c_str(), "r+");
      }
      if (curFile != nullptr) {
        borrowing_info_file = curFile;
      } else {
        cout << "meta can't be open:" << borrowing_info_path.c_str() << "\n";
        return false;
      }
    }
    std::cout << "[PartitionedShareableFileMetaData] Opened Meta files!"
              << std::endl;
    return true;
  }

  /**
   * Initialize the meta partition, with following format:
   * - Metadata Size
   * - FileType
   * - Version
   * - num_shared_pages
   * - sharedPageIds
   *      - NodeID
   *      - SharedPageID
   *      - ...
   */
  int writeMeta() {
    if (upToDate) return 0;

    pthread_mutex_lock(&fileMutex);
    if (borrowing_info_file == nullptr) {
      pthread_mutex_unlock(&fileMutex);
      return -1;
    }
    if (cleared == true) {
      pthread_mutex_unlock(&fileMutex);
      return -1;
    }
    // compute meta size
    size_t metaSize = sizeof(FileType) +
                      sizeof(unsigned short) + // version
                      sizeof(unsigned int) + // total shared pages
                      (sizeof(NodeID) + sizeof(DatabaseID) + sizeof(PageID) + sizeof(SetID)) * sharedPageIds->size(); // sharedPageIds

    // write meta size to meta partition
    fseek(borrowing_info_file, 0, SEEK_SET);
    fwrite((size_t *)(&metaSize), sizeof(size_t), 1, borrowing_info_file);
    fflush(borrowing_info_file);
    // allocate buffer for meta data
    char *buffer = (char *)calloc(metaSize * sizeof(char), 1);
    if (buffer == nullptr) {
      std::cout << "PartitionedShareableFileMetaData.cc: Failed to allocate "
                   "memory with size="
                << metaSize << std::endl;
      exit(1);
    }
    char *cur = buffer;

    // intialize FileType
    *((FileType *)cur) = FileType::PartitionedFileType;
    cur = cur + sizeof(FileType);
    // initialize Version
    *((unsigned short *)cur) = version;
    cur = cur + sizeof(unsigned short);
    // initialize TotalSharedPages
    *((unsigned int *)cur) = sharedPageIds->size();
    cur = cur + sizeof(unsigned int);

    for (int i = 0; i < sharedPageIds->size(); i++) {
      *((NodeID *)cur) = (*sharedPageIds)[i].nodeId;
      cur = cur + sizeof(NodeID);
      *((DatabaseID *)cur) = (*sharedPageIds)[i].dbId;
      cur = cur + sizeof(DatabaseID);
      *((PageID *)cur) = (*sharedPageIds)[i].pageId;
      cur = cur + sizeof(PageID);
      *((SetID *)cur) = (*sharedPageIds)[i].setId;
      cur = cur + sizeof(SetID);
    }

    // write meta data
    fseek(borrowing_info_file, sizeof(size_t), SEEK_SET);
    int ret = writeData(borrowing_info_file, (void *)buffer, metaSize);
    fflush(borrowing_info_file);
    free(buffer);
    pthread_mutex_unlock(&fileMutex);
    std::cout << "[PartitionedShareableFileMetaData] Wrote Meta files!"
              << std::endl;
    if (ret == 0) {
      upToDate = true;
    }
    return ret;
  }

  int writeData(FILE *file, void *data, size_t length) {

    if ((file == nullptr) || (data == nullptr)) {
      cout << "PartitionedFile: Error: writeData with nullptr.\n";
      return -1;
    }
    size_t retSize = fwrite(data, sizeof(char), length, file);
    fflush(file);
    if (retSize != length) {
      return -1;
    } else {
      return 0;
    }
  }

  /**
   * Set up meta data by parsing meta partition
   */
  void buildMetaDataFromMetaPartition() {
    // parse the meta file
    /**
     *  Meta partition format:
     * - Metadata Size
     * - FileType
     * - Version
     * - num_shared_pages
     * - sharedPageIds
     *      - NodeID
     *      - SharedPageID
     *      - ...
     */
    // Open meta partition for reading
    if (openMeta() == false) {
      std::cout
          << "Fatal Error: PartitionedFile: Error: can't open meta partition."
          << std::endl;
      exit(-1);
    }
    // get meta partition size;
    fseek(borrowing_info_file, 0, SEEK_SET);
    size_t size;
    size_t sizeRead =
        fread((size_t *)(&(size)), sizeof(size_t), 1, borrowing_info_file);
    if (sizeRead == 0) {
      std::cout << "PartitionedFile: Read meta size failed" << std::endl;
      exit(-1);
    }
    // load meta partition to memory
    fseek(borrowing_info_file, sizeof(size_t), SEEK_SET);
    char *buf = (char *)calloc(size * sizeof(char), 1);
    if (buf == nullptr) {
      std::cout << "PartitionedFile.cc: Failed to allocate memory with size="
                << size << std::endl;
      exit(1);
    }
    sizeRead = fread((void *)buf, sizeof(char), size, borrowing_info_file);
    if (sizeRead < size) {
      cout << "Metadata corrupted, please remove storage folders and try "
              "again...\n";
      exit(-1);
    }

    // parse file type
    char *cur = buf;
    cur = cur + sizeof(FileType);

    // parse and set version;
    version = (unsigned short)(*(unsigned short *)cur);
    cur = cur + sizeof(unsigned short);

    // parse num_shared_pages
    unsigned int num_shared_pages = (unsigned int)(*(unsigned int *)cur);
    cur = cur + sizeof(unsigned int);

    sharedPageIds->clear();
    assert(sharedPageIds->size() == 0);

    NodeID nodeID;
    DatabaseID dbID;
    PageID pageId;
    SetID setID;
    for (unsigned int i = 0; i < num_shared_pages; i++) {
      nodeID = (NodeID)(*(NodeID *)cur);
      cur = cur + sizeof(NodeID);
      dbID = (DatabaseID)(*(DatabaseID *)cur);
      cur = cur + sizeof(DatabaseID);
      pageId = (PageID)(*(PageID *)cur);
      cur = cur + sizeof(PageID);
      setID = (SetID)(*(SetID *)cur);
      cur = cur + sizeof(SetID);
      addSharedPage(nodeID, dbID, pageId, setID);
    }

    free(buf);
    std::cout << "[PartitionedShareableFileMetaData] Parsed Meta files!"
              << std::endl;
  }

  void addSharedPage(SharedPageID sharedPageID) {
    pthread_mutex_lock(&sharedPageIdsMutex);
    sharedPageIds->push_back(sharedPageID);
    upToDate = false;
    pthread_mutex_unlock(&sharedPageIdsMutex);
  }

  void addSharedPage(NodeID nodeID, DatabaseID dbID, PageID pageId, SetID setID) {
    pthread_mutex_lock(&sharedPageIdsMutex);
    SharedPageID spid;
    spid.nodeId = nodeID;
    spid.dbId = dbID;
    spid.pageId = pageId;
    spid.setId = setID;

    sharedPageIds->push_back(spid);
    upToDate = false;
    pthread_mutex_unlock(&sharedPageIdsMutex);
  }

  vector<SharedPageID> *getSharePageIDs() {
    return sharedPageIds;
  }

  unsigned int getTotalPageCount() {
    return sharedPageIds->size();
  }

  SharedPageID loadSharedPageId(unsigned int numIteratedPages) {
    SharedPageID dummy;
    dummy.dbId = -1;
    dummy.nodeId = -1;
    dummy.pageId = -1;
    dummy.setId = -1;
    if (numIteratedPages >= sharedPageIds->size()) return dummy;
    return sharedPageIds->at(numIteratedPages);
  } 

  void open_shared_files(map<SetID, SetPtr>* typeSets, pdb::PDBLoggerPtr logger, PageCachePtr cache) {
    SharedPageID pid = sharedPageIds->at(0);

    std::cout << "----------------------------------------------------------------------------------" << std::endl;
    if (typeSets->count(pid.setId) == 0) {
      std::cout << "[PartitionedShareableFileMetaData] Referenced Set doesnt exist!" << std::endl;
      return;
    }
    SetPtr set = typeSets->find(pid.setId)->second;
    PageIndex index = set->getFile()->getMetaData()->getPageIndex(pid.pageId);

    PDBPagePtr page = cache->loadPage(set->getFile(), index.partitionId, index.pageSeqInPartition, false);
    Record<Vector<Handle<FFMatrixBlock>>> *myRec1 = (Record<Vector<Handle<FFMatrixBlock>>> *)(page->getBytes());
    Handle<Vector<Handle<FFMatrixBlock>>> inputVec1 = myRec1->getRootObject();

    for (int i = 0; i < inputVec1->size(); i++) {
      std::cout << "GOT MATRIX BLOCK: (" << (*inputVec1)[i]->getBlockRowIndex() << ", " << (*inputVec1)[i]->getBlockColIndex() << ")" << std::endl;
    }

    std::cout << "----------------------------------------------------------------------------------" << std::endl;
  }

  void dump() {
    std::cout << "VERSION: " << version << std::endl;
    for (int i = 0; i < sharedPageIds->size(); i++) {
      std::cout << "NodeID: " << (*sharedPageIds)[i].nodeId << std::endl;
      std::cout << "DBID: " << (*sharedPageIds)[i].dbId << std::endl;
      std::cout << "PageID: " << (*sharedPageIds)[i].pageId << std::endl;
      std::cout << "setID: " << (*sharedPageIds)[i].setId << std::endl;
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
};

#endif /* SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONEDSHAREABLEFILEMETADATA_H_ */
