#ifndef XTZ_CRYPTOGRAPHICHASH_H
#define XTZ_CRYPTOGRAPHICHASH_H

#include "global/icryptographichash.h"

namespace xtz {
class CryptographicHash : public mu::ICryptographicHash
{
public:

    mu::ByteArray hash(const mu::ByteArray& data, Algorithm alg) const override;
};
}

#endif // XTZ_CRYPTOGRAPHICHASH_H
