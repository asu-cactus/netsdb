#include "UserType.h"
#include "PartitionedFile.h"
#include "SharedFFMatrixBlockSet.h"

/**
 * Create a UserType instance.
 */
UserType::UserType(NodeID nodeId,
                   DatabaseID dbId,
                   UserTypeID id,
                   string name,
                   ConfigurationPtr conf,
                   pdb::PDBLoggerPtr logger,
                   SharedMemPtr shm,
                   string metaTypePath,
                   vector<string>* dataTypePaths,
                   PageCachePtr cache,
                   PageCircularBufferPtr flushBuffer) {
    this->name = name;
    this->nodeId = nodeId;
    this->dbId = dbId;
    this->id = id;
    this->conf = conf;
    this->logger = logger;
    this->shm = shm;
    sets = new map<SetID, SetPtr>();
    pthread_mutex_init(&setLock, nullptr);
    this->dataPaths = dataTypePaths;
    unsigned int i;

    this->metaPath = metaTypePath;
    if (this->metaPath.compare("") != 0) {
        this->conf->createDir(this->metaPath);
    }
    for (i = 0; i < this->dataPaths->size(); i++) {
        string dataPath = this->dataPaths->at(i);
        this->conf->createDir(dataPath);
    }
    numSets = 0;
    this->cache = cache;
    this->flushBuffer = flushBuffer;
}

/**
 * Release the in-memory structure that is belonging to a UserType instance.
 * It will not remove the persistent data on disk that is belonging to the UserType.
 */
UserType::~UserType() {
    if (sets != nullptr) {
        delete sets;
    }
    if (dataPaths != nullptr) {
        delete dataPaths;
    }
}

/**
 * Flush UserType data from memory to a PDBFile instance for persistence.
 */
// Now all the flush is managed by PageCache class, so we remove flush here


void UserType::flush() {
}
/**
 * Compute the path to store the UserType data for persistence.
 */
string UserType::encodePath(string typePath, SetID setId, string setName) {
    char buffer[500];
    sprintf(buffer, "%s/%d_%s", typePath.c_str(), setId, setName.c_str());
    return string(buffer);
}

// add new set
// Not thread-safe
int UserType::addSet(string setName, SetID setId, size_t pageSize, size_t desiredSize, bool isMRU, bool isTransient, bool isSharedFFMatrixBlockSet) {
    if (this->sets->find(setId) != this->sets->end()) {
        this->logger->writeLn("UserType: set exists.");
        return -1;
    }
    string typePath;
    PartitionedFilePtr file;
    string metaFilePath = this->encodePath(this->metaPath, setId, setName);
    vector<string> dataFilePaths;
    unsigned int i;
    for (i = 0; i < this->dataPaths->size(); i++) {
        dataFilePaths.push_back(this->encodePath(this->dataPaths->at(i), setId, setName));
    }
    file = make_shared<PartitionedFile>(this->nodeId,
                                        this->dbId,
                                        this->id,
                                        setId,
                                        metaFilePath,
                                        dataFilePaths,
                                        this->logger,
                                        pageSize);

    SetPtr set = nullptr;
    LocalitySetReplacementPolicy policy = LRU;
    if (isMRU) {
        policy = MRU;
    }
    PersistenceType persistenceType = Persistent;
    if (isTransient) {
        persistenceType = Transient;
    }

    if (isSharedFFMatrixBlockSet) {
        std::cout << "****************We are creating a SharedFFMatrixBlockSet instance*********************" << std::endl;
    	set = make_shared<pdb::SharedFFMatrixBlockSet>(pageSize, logger, shm, nodeId, dbId, id, setId, setName, file, this->cache, JobData, policy, Write, TryCache, persistenceType, desiredSize);
    } else {
        set = make_shared<UserSet>(
          pageSize, logger, shm, nodeId, dbId, id, setId, setName, file, this->cache, JobData, policy, Write, TryCache, persistenceType, desiredSize);
    }
    if (set == 0) {
        this->logger->writeLn("UserType: Out of Memory.");
        return -1;
    }
    this->logger->writeLn("UserType: set added.");
    pthread_mutex_lock(&setLock);
    this->sets->insert(pair<SetID, SetPtr>(setId, set));
    pthread_mutex_unlock(&setLock);
    this->numSets++;
    return 0;
}

// Remove an existing set.
// If successful, return 0.
// Otherwise, e.g. the set doesn't exist, return -1.
int UserType::removeSet(SetID setId) {
    map<SetID, SetPtr>::iterator setIter;
    if ((setIter = this->sets->find(setId)) != this->sets->end()) {
        this->logger->writeLn("UserType: removing input buffer for set:");
        this->logger->writeInt(setId);
        this->logger->writeLn("\n");
        setIter->second->getFile()->clear();
        pthread_mutex_lock(&setLock);
        this->sets->erase(setIter);
        pthread_mutex_unlock(&setLock);
        this->logger->writeLn("UserType: set is removed.");
        this->numSets--;
        return 0;
    }
    this->logger->writeLn("UserType: set doesn't exist.");
    return -1;
}

// Return the specified set that is belonging to this type instance.
SetPtr UserType::getSet(SetID setId) {
    map<SetID, SetPtr>::iterator setIter;
    if ((setIter = sets->find(setId)) != sets->end()) {
        return setIter->second;
    } else {
        return nullptr;
    }
}

using namespace boost::filesystem;

// Initialize type instance based on disk dirs and files.
// This function is only used for PartitionedFile instances.
bool UserType::initializeFromMetaTypeDir(path metaTypeDir) {
    std::cout << "metaTypeDir is " << metaTypeDir << std::endl; 
    // traverse all set files in type directory
    // for each set file, invoke addSet to initialize PDBFile object and CircularInputBuffer
    if (exists(metaTypeDir)) {
        if (is_directory(metaTypeDir)) {
            vector<path> metaSetFiles;
            copy(
                directory_iterator(metaTypeDir), directory_iterator(), back_inserter(metaSetFiles));
            vector<path>::iterator iter;
            std::string path;
            std::string dirName;
            std::string name;
            SetID setId;
            for (iter = metaSetFiles.begin(); iter != metaSetFiles.end(); iter++) {
                if (is_regular_file(*iter)) {
                    // find a set
                    path = std::string(iter->c_str());
                    std::cout << "path to set file is " << path << std::endl;
                    dirName = path.substr(path.find_last_of('/') + 1, path.length() - 1);
                    // parse set name
                    name = dirName.substr(dirName.find('_') + 1, dirName.length() - 1);
                    // parse set id
                    setId = stoul(dirName.substr(0, dirName.find('_')));
                    std::cout << "set name is " << name << ", setId is " << setId << std::endl;
                    // check whether set exists
                    if (this->sets->find(setId) != this->sets->end()) {
                        this->logger->writeLn("UserType: set exists.");
                        return false;
                    }

                    std::cout << "create partitioned file instance" << std::endl;
                    // create PartitionedFile instance
                    PartitionedFilePtr partitionedFile = make_shared<PartitionedFile>(
                        this->nodeId, this->dbId, this->id, setId, path, this->logger);

                    std::cout << "build metadata from meta partition" << std::endl;
                    partitionedFile->buildMetaDataFromMetaPartition(nullptr);
                    std::cout << "initialize data files" << std::endl;
                    partitionedFile->initializeDataFiles();
                    std::cout << "open data" << std::endl;
                    partitionedFile->openData();
                    std::cout << "create set" << std::endl;
                    // create a Set instance from file
                    SetPtr set = make_shared<UserSet>(partitionedFile->getPageSize(),
                                                      logger,
                                                      this->shm,
                                                      nodeId,
                                                      dbId,
                                                      id,
                                                      setId,
                                                      name,
                                                      partitionedFile,
                                                      this->cache, JobData, LRU, Read, TryCache, Persistent);
                    // add buffer to map
                    if (set == 0) {
                        this->logger->error("Fatal Error: UserType: out of memory.");
                        exit(1);
                    }
                    this->sets->insert(pair<SetID, SetPtr>(setId, set));
                    this->numSets++;
                    this->logger->writeLn("UserType: set added.");
                }
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

// Initialize type instance based on disk dirs and files.
// This function is used for importing sets from SequenceFile instances.
// This function is mainly to provide backward compatibility for SequenceFile instances.
bool UserType::initializeFromTypeDir(path typeDir) {

    return false;
}
