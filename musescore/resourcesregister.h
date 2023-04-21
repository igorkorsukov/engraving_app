#ifndef XTZ_IO_RESOURCESREGISTER_H
#define XTZ_IO_RESOURCESREGISTER_H

#include <vector>
#include <string>
#include <map>

#include "global/io/path.h"
#include "global/types/bytearray.h"

#define INIT_RESOURCE(name) \
    extern void InitResources_##name(); \
    InitResources_##name(); \

namespace xtz::io {
class ResourcesRegister
{
public:

    static ResourcesRegister* instance();

    void addData(const std::vector<std::string>& files, mu::ByteArray data);

    bool exists(const std::string& filePath);
    bool readFile(const std::string& filePath, mu::ByteArray& fileData);
    mu::io::paths_t scanFiles(const std::string& path);

    void destroy();

private:

    std::map<std::string, mu::ByteArray> m_data;
};
}

#endif // XTZ_IO_RESOURCESREGISTER_H
