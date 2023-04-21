#include "resourcesregister.h"

#include "global/serialization/zipreader.h"
#include "global/io/buffer.h"

#include "log.h"

using namespace xtz::io;
using namespace mu;

namespace rc {
void RegisterResourceData(const std::vector<std::string>& files, const uint8_t* data, const size_t dataSize)
{
    ResourcesRegister::instance()->addData(files, ByteArray::fromRawData(data, dataSize));
}
}

static bool startsWith(const std::string& str, const std::string& start)
{
    if (start.size() > str.size()) {
        return false;
    }

    for (size_t i = 0; i < start.size(); ++i) {
        if (str.at(i) == start.at(i)) {
            continue;
        }
        return false;
    }
    return true;
}

static std::string remove_sheme(const std::string& path)
{
    if (path.size() < 3) {
        return std::string();
    }

    if (path.at(0) == ':') {
        return path.substr(2);
    }
    return path;
}

ResourcesRegister* ResourcesRegister::instance()
{
    static ResourcesRegister rr;
    return &rr;
}

void ResourcesRegister::addData(const std::vector<std::string>& files, ByteArray data)
{
    for (size_t i = 0; i < files.size(); ++i) {
        std::string file = ":/" + files.at(i);
        IF_ASSERT_FAILED(m_data.find(file) == m_data.end()) {
            continue;
        }

        m_data[file] = data;
    }
}

void ResourcesRegister::destroy()
{
    m_data.clear();
}

bool ResourcesRegister::exists(const std::string& filePath)
{
    return m_data.find(filePath) != m_data.end();
}

bool ResourcesRegister::readFile(const std::string& filePath, ByteArray& fileData)
{
    auto it = m_data.find(filePath);
    if (it == m_data.end()) {
        LOGE() << "not found file: " << filePath;
        return false;
    }

    ByteArray& zipData = it->second;
    mu::io::Buffer buf(&zipData);

    mu::ZipReader zip(&buf);
    fileData = zip.fileData(remove_sheme(filePath));

    if (fileData.empty()) {
        LOGW() << "the file is empty, may not be found, path: " << filePath;
        LOGI() << "has files: ";
        std::vector<ZipReader::FileInfo> files = zip.fileInfoList();
        for (const ZipReader::FileInfo& fi : files) {
            LOGI() << "    " << fi.filePath;
        }
    }

    return !zip.hasError();
}

mu::io::paths_t ResourcesRegister::scanFiles(const std::string& rootPath)
{
    mu::io::paths_t paths;
    size_t relStart = rootPath.back() == '/' ? rootPath.size() : (rootPath.size() + 1);
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        const std::string& filePath = it->first;
        if (!startsWith(filePath, rootPath)) {
            continue;
        }

        std::string relativeFilePath = filePath.substr(relStart);

        //! NOTE At the moment we are implementing a scan of only the current directory
        if (relativeFilePath.find('/') != std::string::npos) {
            continue;
        }

        paths.push_back(relativeFilePath);
    }

    return paths;
}
