#ifndef PDB_UTILITIES_SAFERESULT_H
#define PDB_UTILITIES_SAFERESULT_H

#include <functional>
#include <memory>
#include <string>
#include <stdlib.h>

using std::function;
using std::string;
using std::shared_ptr;

namespace pdb {

template <typename P>
class SafeResult {

public:
    virtual void apply(function<void(P)> forSuccessCase,
                       function<void(string errorMsg)> forErrorCase) = 0;

    /**
     * If this safe result is in the success case, returns the result item.  Otherwise calls
     * exit(exitCode)
     * and terminates the process.
     */
    P getResultOrExit(int exitCode) {
        P* result;
        this->apply([&](P successResult) { result = &successResult; },

                    [&](string errorMessage) { exit(exitCode); });

        return *result;
    }
};

template <typename P>
class SafeResultSuccess : public SafeResult<P> {

public:
    SafeResultSuccess(P result) : _result(result) {}

    void apply(function<void(P)> forSuccessCase, function<void(string)> /*forFailureCase*/) {
        return forSuccessCase(_result);
    }

private:
    P _result;
};

template <typename P>
class SafeResultFailure : public SafeResult<P> {
public:
    SafeResultFailure(string errorMessage) {
        _errorMessage = errorMessage;
    }

    void apply(function<void(P)> /*forSuccessCase*/, function<void(string)> forFailureCase) {
        return forFailureCase(_errorMessage);
    }

private:
    string _errorMessage;
};
}

#endif  // ALLUNITTESTS_SAFERESULT_H
