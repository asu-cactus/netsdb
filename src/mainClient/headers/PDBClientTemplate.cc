#ifndef PDB_CLIENT_TEMPLATE_CC
#define PDB_CLIENT_TEMPLATE_CC

#include "PDBClient.h"

namespace pdb {

template <class DataType>
bool PDBClient::createSet(const std::string &databaseName,
                          const std::string &setName, std::string &errMsg, 
                          size_t pageSize, const std::string &createdJobId, Handle<Computation> dispatchComputation, Handle<LambdaIdentifier> lambda, bool isSharedTensorBlockSet) {

  return distributedStorageClient.createSet<DataType>(
        databaseName, setName, errMsg, pageSize, createdJobId, dispatchComputation, lambda, isSharedTensorBlockSet);
}



template <class DataType>
bool PDBClient::createSet(const std::string &databaseName,
                          const std::string &setName, std::string &errMsg,
                          size_t pageSize, const std::string &createdJobId, Handle<Vector<Handle<Computation>>> dispatchComputations, std::string jobName, std::string jobName1, std::string jobName2, std::string computationName1, std::string computationName2, std::string lambdaName1, std::string lambdaName2, bool isSharedTensorBlockSet) {

  return distributedStorageClient.createSet<DataType>(
        databaseName, setName, errMsg, pageSize, createdJobId, dispatchComputations, jobName, jobName1, jobName2, computationName1, computationName2, lambdaName1, lambdaName2, isSharedTensorBlockSet);
}



template <class DataType>
bool PDBClient::sendData(std::pair<std::string, std::string> setAndDatabase,
                         Handle<Vector<Handle<DataType>>> dataToSend,
                         std::string &errMsg) {

  return dispatcherClient.sendData<DataType>(setAndDatabase, dataToSend,
                                             errMsg);
}

template <class... Types>
bool PDBClient::executeComputations(std::string &errMsg,
                                    Handle<Computation> firstParam,
                                    Handle<Types>... args) {
  return queryClient.executeComputations(errMsg, firstParam, args...);
}


template <class... Types>
bool PDBClient::executeComputations(std::string &errMsg,
                                    std::string jobName,
                                    Handle<Computation> firstParam,
                                    Handle<Types>... args) {
  return queryClient.executeComputations(errMsg, jobName, firstParam, args...);
}



template <class... Types>
bool PDBClient::executeComputations(std::string &errMsg,
		                    std::string jobName,
                                    bool preCompile,
                                    Handle<Computation> firstParam,
                                    Handle<Types>... args) {
  return queryClient.executeComputations(errMsg, jobName, preCompile, firstParam, args...);
}





template <class Type>
SetIterator<Type> PDBClient::getSetIterator(std::string databaseName,
                                            std::string setName, 
					    bool isShared) {
  return queryClient.getSetIterator<Type>(databaseName, setName, isShared);
}
}
#endif
