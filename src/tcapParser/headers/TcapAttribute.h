#ifndef PDB_TCAPPARSER_TCAPATTRIBUTE_H
#define PDB_TCAPPARSER_TCAPATTRIBUTE_H

#include "TcapIdentifier.h"
#include "StringLiteral.h"

#include <memory>

using std::shared_ptr;

namespace pdb_detail {
/**
 * A key value pair associated with a statement.
 */
class TcapAttribute {
public:
    /**
     * The key name of the attribute.
     */
    const TcapIdentifier name;

    /**
     * The value of the attribute.
     */
    const StringLiteral value;

    /**
     * Creates a new TCAP attribute.
     *
     * @param name The key name of the attribute.
     * @param value The value of the attribute.
     * @return a new TcapAttribute
     */
    TcapAttribute(const TcapIdentifier& name, const StringLiteral& value);
};

typedef shared_ptr<TcapAttribute> TcapAttributePtr;
}

#endif  // PDB_TCAPATTRIBUTE_H
