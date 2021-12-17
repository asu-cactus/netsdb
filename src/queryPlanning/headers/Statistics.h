#ifndef PDB_STATISTICS_H
#define PDB_STATISTICS_H

#include <memory>
#include <pthread.h>
#include <unordered_map>

namespace pdb {

class Statistics;
typedef std::shared_ptr<Statistics> StatisticsPtr;

// A class to encapsulate set statistics for physical scheduling

struct DataStatistics {

  std::string databaseName;
  std::string setName;
  int numPages = 0;
  size_t pageSize = 0;
  size_t numBytes = 0;
  int numTuples = 0;
  size_t avgTupleSize = 0;
};

class Statistics {

private:
  std::unordered_map<std::string, DataStatistics> dataStatistics;
  std::unordered_map<std::string, double> atomicComputationSelectivity;
  std::unordered_map<std::string, double> lambdaSelectivity;
  std::unordered_map<std::string, double> penalizedCosts;
  pthread_mutex_t mutex;

public:
  // constructor
  Statistics() { pthread_mutex_init(&mutex, nullptr); }

  // destructor
  ~Statistics() { pthread_mutex_destroy(&mutex); }

  // add penalized set and cost
  void setPenalizedCost(std::string setName, double cost) {
    penalizedCosts[setName] = cost;
  }

  // remove penalized set
  void removePenalizedCost(std::string setName) {
    if (penalizedCosts.count(setName) > 0) {
      penalizedCosts.erase(setName);
    }
  }

  // to return penalized cost
  double getPenalizedCost(std::string setName) {
    if (penalizedCosts.count(setName) > 0) {
      return penalizedCosts[setName];
    } else {
      return -1;
    }
  }

  // to clear penalized costs
  void clearPenalizedCosts() {
    penalizedCosts.clear();
  }

  // remove set
  void removeSet(std::string databaseName, std::string setName) {
    std::string key = databaseName + ":" + setName;
    if (dataStatistics.count(key) > 0) {
      dataStatistics.erase(key);
    }
  }

  // to return number of pages of a set
  int getNumPages(std::string databaseName, std::string setName) {
    std::string key = databaseName + ":" + setName;
    if (dataStatistics.count(key) == 0) {
      return 0;
    } else {
      return dataStatistics[key].numPages;
    }
  }

  // to set number of pages of a set
  void setNumPages(std::string databaseName, std::string setName,
                   int numPages) {
    std::string key = databaseName + ":" + setName;
    pthread_mutex_lock(&mutex);
    dataStatistics[key].numPages = numPages;
    pthread_mutex_unlock(&mutex);
  }

  // to increment number of pages of a set
  void incrementNumPages(std::string databaseName, std::string setName,
                         int numPages) {
    std::string key = databaseName + ":" + setName;
    pthread_mutex_lock(&mutex);
    dataStatistics[key].numPages += numPages;
    pthread_mutex_unlock(&mutex);
    std::cout << "number of pages in " << key << " is " << dataStatistics[key].numPages << std::endl;
  }



  // to return page size of a set
  size_t getPageSize(std::string databaseName, std::string setName) {
    std::string key = databaseName + ":" + setName;
    if (dataStatistics.count(key) == 0) {
      return -1;
    } else {
      return dataStatistics[key].pageSize;
    }
  }

  // to set page size of a set
  void setPageSize(std::string databaseName, std::string setName,
                   size_t pageSize) {
    std::string key = databaseName + ":" + setName;
    pthread_mutex_lock(&mutex);
    dataStatistics[key].pageSize = pageSize;
    pthread_mutex_unlock(&mutex);
  }

  // to return numBytes of a set
  size_t getNumBytes(std::string databaseName, std::string setName) {
    std::string key = databaseName + ":" + setName;
    if (dataStatistics.count(key) == 0) {
      return 0;
    } else {
      return dataStatistics[key].numBytes;
    }
  }

  // to set numBytes of a set
  void setNumBytes(std::string databaseName, std::string setName,
                   size_t numBytes) {
    std::string key = databaseName + ":" + setName;
    pthread_mutex_lock(&mutex);
    dataStatistics[key].numBytes = numBytes;
    pthread_mutex_unlock(&mutex);
  }

  // to increment number of bytes of a set
  void incrementNumBytes(std::string databaseName, std::string setName,
                         size_t numBytes) {
    std::string key = databaseName + ":" + setName;
    pthread_mutex_lock(&mutex);
    dataStatistics[key].numBytes += numBytes;
    std::cout << "we increment numBytes to " << dataStatistics[key].numBytes << std::endl;
    pthread_mutex_unlock(&mutex);

  }

  // to return number of tuples of a set
  int getNumTuples(std::string databaseName, std::string setName) {
    std::string key = databaseName + ":" + setName;
    if (dataStatistics.count(key) == 0) {
      return -1;
    } else {
      return dataStatistics[key].numTuples;
    }
  }

  // to set number of tuples of a set
  void setNumTuples(std::string databaseName, std::string setName,
                    int numTuples) {
    std::string key = databaseName + ":" + setName;
    pthread_mutex_lock(&mutex);
    dataStatistics[key].numTuples = numTuples;
    pthread_mutex_unlock(&mutex);
  }

  // to return average tuple size of a set
  size_t getAvgTupleSize(std::string databaseName, std::string setName) {
    std::string key = databaseName + ":" + setName;
    if (dataStatistics.count(key) == 0) {
      return -1;
    } else {
      return dataStatistics[key].avgTupleSize;
    }
  }

  // to set average tuple size of a set
  void setAvgTupleSize(std::string databaseName, std::string setName,
                       size_t avgTupleSize) {
    std::string key = databaseName + ":" + setName;
    pthread_mutex_lock(&mutex);
    dataStatistics[key].avgTupleSize = avgTupleSize;
    pthread_mutex_unlock(&mutex);
  }

  // to return selectivity of an atomic computation
  double getAtomicComputationSelectivity(std::string atomicComputationType) {
    if (atomicComputationSelectivity.count(atomicComputationType) == 0) {
      return 0;
    } else {
      return atomicComputationSelectivity[atomicComputationType];
    }
  }

  // to set selectivity for an atomic computation
  void setAtomicComputationSelectivity(std::string atomicComputationType,
                                       double selectivity) {
    pthread_mutex_lock(&mutex);
    atomicComputationSelectivity[atomicComputationType] = selectivity;
    pthread_mutex_unlock(&mutex);
  }

  // to return selectivity of a lambda
  double getLambdaSelectivity(std::string lambdaType) {
    if (lambdaSelectivity.count(lambdaType) == 0) {
      return 0;
    } else {
      return lambdaSelectivity[lambdaType];
    }
  }

  // to set selectivity for a lambda
  void setLambdaSelectivity(std::string lambdaType, double selectivity) {
    pthread_mutex_lock(&mutex);
    lambdaSelectivity[lambdaType] = selectivity;
    pthread_mutex_unlock(&mutex);
  }
};
}

#endif
