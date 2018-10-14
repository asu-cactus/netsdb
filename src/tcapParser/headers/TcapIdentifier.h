#ifndef PDB_TCAPPARSER_TCAPIDENTIFIER_H
#define PDB_TCAPPARSER_TCAPIDENTIFIER_H

#include <memory>
#include <string>

using std::shared_ptr;
using std::string;

namespace pdb_detail {

/**
 * A TCAP identifier.
 */
class TcapIdentifier {

public:
    /**
     * The value of the identifier.
     */
    const string contents;

    /**
     * Creates a new TCAP identifier.
     *
     * @param contents the value of the identifier.
     * @return a new TCAP identifier.
     */
    TcapIdentifier(const string& contents);
};
}

#endif  // PDB_TCAPPARSER_TCAPIDENTIFIER_H
