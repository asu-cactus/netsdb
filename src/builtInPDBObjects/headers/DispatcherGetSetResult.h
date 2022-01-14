#ifndef DISPATCHERGETSETRESULT_H
#define DISPATCHERGETSETRESULT_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include <utility>

// PRELOAD %DispatcherGetSetResult%

namespace pdb {

    /**
     * Encapsulates a request to search for a set in the database
     */

    class DispatcherGetSetResult : public Object {

    public:

        DispatcherGetSetResult(){}
        ~DispatcherGetSetResult(){}

        /**
         * Creates a request to get the database
         * @param database - the name of database
         */

        explicit DispatcherGetSetResult(const std::string &database, const std::string &set, std::string fileName, std::string errMsg) : databaseName(database), setName(set), fileName(fileName), errMsg(errMsg) {}

        ENABLE_DEEP_COPY

        /**
         * The name of the database
         */

        String databaseName;

        /**
         * The name of the set
         */

        String setName;

        string fileName;

        bool status;

        string errMsg;

        void setStatus(bool status) {
            this->status = status;
        }

        bool getStatus() {
            return status;
        }

        void setFileName(std::string fileName) {
            this->fileName = fileName;
        }

        void setDatabaseName(std::string databaseName) {
            this->databaseName = databaseName;
        }

        void setSetName(std::string setName) {
            this->setName = setName;
        }

        std::string getFileName() {
            return fileName;
        }

        std::string getDatabaseName() {
            return databaseName;
        }

        std::string getSetName() {
            return setName;
        }

        std::pair<std::string, std::string> getRes() {
            return std::make_pair(fileName, errMsg);
        }
    };
}

#endif