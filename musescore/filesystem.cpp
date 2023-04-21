#include "filesystem.hpp"

#include <cstring>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>

#include <global/io/ioretcodes.h>
#include "resourcesregister.h"

#include "log.h"

using namespace xtz::io;
using namespace mu;
using namespace mu::io;

static bool isFsAvalable()
{
#ifdef PLATFORM_WEB
    return false;
#else
    return true;
#endif
}

Ret FileSystem::exists(const path_t& path) const
{
    if (isResourcePath(path)) {
        return resourceExists(path);
    }

    if (!isFsAvalable()) {
        LOGW() << "fs unavailable, path: " << path;
        return make_ret(Ret::Code::NotSupported);
    }

    std::ifstream f(path.c_str());
    if (f.good()) {
        return make_ret(Err::NoError);
    }

    return make_ret(Err::FSNotExist);
}

Ret FileSystem::remove(const path_t& path, bool onlyIfEmpty)
{
    if (!isFsAvalable()) {
        LOGW() << "fs unavailable, path: " << path;
        return make_ret(Ret::Code::NotSupported);
    }

    struct stat status = {};
    if (stat(path.c_str(), &status) == -1) {
        // not exists
        return make_ret(Err::NoError);
    }

    return S_ISDIR(status.st_mode) ? removeDir(path) : removeFile(path);

    return make_ret(Err::NoError);
}

Ret FileSystem::clear(const path_t& path)
{
    UNUSED(path);
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

mu::Ret FileSystem::removeDir(const mu::io::path_t& path) const
{
    UNUSED(path);
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

mu::Ret FileSystem::removeFile(const mu::io::path_t& path) const
{
    bool ok = std::remove(path.c_str()) == 0;
    return make_ret(ok ? Err::NoError : Err::FSRemoveError);
}

Ret FileSystem::copy(const path_t& src, const path_t& dst, bool replace)
{
    UNUSED(src);
    UNUSED(dst);
    UNUSED(replace);
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

Ret FileSystem::move(const path_t& src, const path_t& dst, bool replace)
{
    UNUSED(src);
    UNUSED(dst);
    UNUSED(replace);
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

Ret FileSystem::makePath(const path_t& path) const
{
    if (!isFsAvalable()) {
        LOGW() << "fs unavailable, path: " << path;
        return make_ret(Ret::Code::NotSupported);
    }

    struct stat status = {};
    if (stat(path.c_str(), &status) != -1) {
        return true;
    }
    return mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}

EntryType FileSystem::entryType(const mu::io::path_t& path) const
{
    // unused
    return EntryType::Undefined;
}

RetVal<uint64_t> FileSystem::fileSize(const path_t& path) const
{
    if (!isFsAvalable()) {
        LOGW() << "fs unavailable, path: " << path;
        RetVal<uint64_t> rv;
        rv.val = 0;
        rv.ret = make_ret(Ret::Code::NotSupported);
        return rv;
    }

    RetVal<uint64_t> rv;

    std::ifstream in(path.toStdString(), std::ifstream::ate | std::ifstream::binary);
    uint64_t size = in.tellg();

    rv.val = static_cast<uint64_t>(size);
    rv.ret = make_ok();
    return rv;
}

RetVal<paths_t> FileSystem::scanFiles(const path_t& rootDir, const std::vector<std::string>& filters, ScanMode mode) const
{
    UNUSED(filters);
    UNUSED(mode);

    if (isResourcePath(rootDir)) {
        return resourceScanFiles(rootDir);
    }

    if (!isFsAvalable()) {
        LOGW() << "fs unavailable, path: " << rootDir;
        return RetVal<paths_t>(make_ret(Ret::Code::NotSupported));
    }

    RetVal<paths_t> ret;

    struct dirent** namelist;
    int n = scandir(rootDir.c_str(), &namelist, NULL, NULL);
    if (n == -1) {
        ret.ret = make_ret(Err::UnknownError);
        return ret;
    }

    ret.ret = make_ok();

    static const unsigned char DIR_TYPE = 4;
    static const unsigned char FILE_TYPE = 8;

    while (n--) {
        if (std::strcmp(namelist[n]->d_name, ".") == 0 || std::strcmp(namelist[n]->d_name, "..") == 0) {
            continue;
        }

        switch (mode) {
        case ScanMode::FilesInCurrentDir: {
            if (namelist[n]->d_type == FILE_TYPE) {
                ret.val.push_back(rootDir + "/" + path_t(namelist[n]->d_name));
            }
        } break;
        case ScanMode::FilesAndFoldersInCurrentDir: {
            ret.val.push_back(rootDir + "/" + path_t(namelist[n]->d_name));
        } break;
        case ScanMode::FilesInCurrentDirAndSubdirs: {
            if (namelist[n]->d_type == FILE_TYPE) {
                ret.val.push_back(rootDir + "/" + path_t(namelist[n]->d_name));
            } else if (namelist[n]->d_type == DIR_TYPE) {
                RetVal<paths_t> rv = scanFiles(rootDir + "/" + path_t(namelist[n]->d_name), filters, mode);
                if (rv.ret) {
                    ret.val.insert(ret.val.end(), rv.val.begin(), rv.val.end());
                }
            }
        } break;
        }

        free(namelist[n]);
    }
    free(namelist);

    return ret;
}

RetVal<ByteArray> FileSystem::readFile(const path_t& filePath) const
{
    RetVal<ByteArray> rv;
    ByteArray data;

    if (!readFile(filePath, data)) {
        rv.ret = make_ret(Err::FSReadError);
        return rv;
    }

    rv.val = data;
    rv.ret = make_ok();
    return rv;
}

bool FileSystem::readFile(const path_t& filePath, ByteArray& data) const
{
    if (!exists(filePath)) {
        LOGE() << "failed read, not exists: " << filePath;
        return false;
    }

    if (isResourcePath(filePath)) {
        return resourceReadFile(filePath, data);
    }

    if (!isFsAvalable()) {
        LOGW() << "fs unavailable, path: " << filePath;
        return false;
    }

    std::ifstream ifs(filePath.c_str(), std::ios::binary);

    size_t size = static_cast<size_t>(fileSize(filePath).val);
    data.resize(size);

    if (!ifs.read((char*)data.data(), data.size())) {
        LOGE() << "failed read: " << filePath;
        return false;
    }

    return true;
}

Ret FileSystem::writeFile(const path_t& filePath, const ByteArray& data) const
{
    if (!isFsAvalable()) {
        LOGW() << "fs unavailable, path: " << filePath;
        return make_ret(Ret::Code::NotSupported);
    }

    FILE* file = fopen(filePath.c_str(), "wb");
    if (!file) {
        LOGE() << "Failed to open to writing file: " << filePath;
        return make_ret(Err::FSWriteError);
    }

    fwrite(data.constData(), 1, data.size(), file);

    fclose(file);

    return make_ok();
}

void FileSystem::setAttribute(const path_t& path, Attribute attribute) const
{
    UNUSED(path);
    UNUSED(attribute);
}

bool FileSystem::setPermissionsAllowedForAll(const path_t& path) const
{
    UNUSED(path);
    return true;
}

mu::io::path_t FileSystem::canonicalFilePath(const path_t& filePath) const
{
    return filePath;
}

mu::io::path_t FileSystem::absolutePath(const path_t& filePath) const
{
    path_t fullPath = filePath;
    if (isResourcePath(filePath)) {
        return filePath;
    }

    String strPath = filePath.toString();
    size_t lastSep = strPath.lastIndexOf(u'/');
    size_t lastDot = strPath.lastIndexOf(u'.');
    if (lastDot > lastSep) {
        //!@NOTE remove filename from fullpath
        fullPath = strPath.remove(lastSep);
    }

    return fullPath;
}

path_t FileSystem::absoluteFilePath(const path_t& filePath) const
{
    if (isResourcePath(filePath)) {
        return filePath;
    }

    return filePath;
}

DateTime FileSystem::birthTime(const path_t& filePath) const
{
    UNUSED(filePath);
    NOT_SUPPORTED;
    return DateTime();
}

DateTime FileSystem::lastModified(const path_t& filePath) const
{
    UNUSED(filePath);
    NOT_SUPPORTED;
    return DateTime();
}

bool FileSystem::isWritable(const path_t&) const
{
    return false;
}

bool FileSystem::isResourcePath(const path_t& path) const
{
    if (path.empty()) {
        return false;
    }

    if (path.c_str()[0] == ':') {
        return true;
    }

    return false;
}

bool FileSystem::resourceExists(const mu::io::path_t& path) const
{
    return ResourcesRegister::instance()->exists(path.toStdString());
}

bool FileSystem::resourceReadFile(const mu::io::path_t& filePath, mu::ByteArray& data) const
{
    return ResourcesRegister::instance()->readFile(filePath.toStdString(), data);
}

mu::RetVal<mu::io::paths_t> FileSystem::resourceScanFiles(const mu::io::path_t& path) const
{
    mu::RetVal<mu::io::paths_t> rv;
    rv.ret = make_ok();
    rv.val = ResourcesRegister::instance()->scanFiles(path.toStdString());
    return rv;
}
