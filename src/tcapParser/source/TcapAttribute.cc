#include "TcapAttribute.h"

namespace pdb_detail {
TcapAttribute::TcapAttribute(const TcapIdentifier& name, const StringLiteral& value)
    : name(name), value(value) {}
}
