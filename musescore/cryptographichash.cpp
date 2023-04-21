#include "cryptographichash.h"

#include <string>

using namespace xtz;

mu::ByteArray CryptographicHash::hash(const mu::ByteArray& data, Algorithm) const
{
    std::string str = std::to_string(data.size());
    return mu::ByteArray(&str[0], str.size());
}
