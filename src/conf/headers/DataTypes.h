
#ifndef PDB_DATA_TYPES_H
#define PDB_DATA_TYPES_H

#include <cstddef>

typedef unsigned int PageIteratorsID;
typedef unsigned int UserTypeID;
typedef unsigned int PageID;
typedef unsigned int NodeID;
typedef unsigned int HashPartitionID;
typedef unsigned int DatabaseID;
typedef unsigned int ServerID;
typedef unsigned int SetID;
typedef unsigned int FilePartitionID;
typedef size_t MemPoolOffset;
typedef size_t PageOffset;
typedef unsigned long long ticks_t;
typedef unsigned int OperatorID;
typedef unsigned int JobStageID;

// Priority level for keeping in memory
// from low to high
typedef enum {
    TransientLifetimeEnded,
    PersistentLifetimeEnded,
    PersistentLifetimeNotEnded,
    TransientLifetimeNotEndedPartialData,
    TransientLifetimeNotEndedShuffleData,
    TransientLifetimeNotEndedHashData
} PriorityLevel;

typedef enum { JobData, ShuffleData, HashPartitionData, PartialAggregationData } LocalityType;

typedef enum { LRU, MRU, Random } LocalitySetReplacementPolicy;

typedef enum { UnifiedLRU, UnifiedMRU, UnifiedCost, UnifiedIntelligent, UnifiedDBMIN } CacheStrategy;


typedef enum { Read, RepeatedRead, Write } OperationType;

typedef enum { TryCache, CacheThrough } DurabilityType;

typedef enum { FixedSize, VariableSize } SmallPageType;

typedef enum {
    StraightSequential,
    LoopingSequential,
    SmallSequential

} AccessPattern;

typedef enum { Transient, Persistent } PersistenceType;

typedef enum { Direct, Recreation, DeepCopy } ObjectCreationMode;

typedef enum { SequenceFileType, PartitionedFileType } FileType;

typedef enum { PeriodicTimer, OneshotTimer } TimerType;

typedef enum {
    UserSetType,
    SharedHashSetType,  // the input for first phase scan join, or the input for second phase
                        // probing of shuffle join
    PartitionedHashSetType,  // the input for second phase of shuffle join and aggregation
} SetType;


typedef struct {
    DatabaseID dbId;
    UserTypeID typeId;
    SetID setId;
    PageID pageId;
} CacheKey;

typedef struct {
    bool inCache;
    FilePartitionID partitionId;
    unsigned int pageSeqInPartition;
} FileSearchKey;

typedef struct {
    FilePartitionID partitionId;
    unsigned int pageSeqInPartition;
} PageIndex;

typedef struct {
    DatabaseID dbId;
    UserTypeID typeId;
    SetID setId;
} SetKey;

#endif
