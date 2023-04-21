#ifndef XTZ_IO_FILESYSTEMBASE_H
#define XTZ_IO_FILESYSTEMBASE_H

#include <global/io/ifilesystem.h>

namespace xtz::io {
class FileSystem : public mu::io::IFileSystem
{
public:
    mu::Ret exists(const mu::io::path_t& path) const override;
    mu::Ret remove(const mu::io::path_t& path, bool onlyIfEmpty = false) override;
    mu::Ret clear(const mu::io::path_t& path) override;
    mu::Ret copy(const mu::io::path_t& src, const mu::io::path_t& dst, bool replace = false) override;
    mu::Ret move(const mu::io::path_t& src, const mu::io::path_t& dst, bool replace = false) override;

    mu::Ret makePath(const mu::io::path_t& path) const override;
    mu::io::EntryType entryType(const mu::io::path_t& path) const override;

    mu::RetVal<uint64_t> fileSize(const mu::io::path_t& path) const override;

    mu::RetVal<mu::io::paths_t> scanFiles(const mu::io::path_t& rootDir, const std::vector<std::string>& filters,
                                          mu::io::ScanMode mode = mu::io::ScanMode::FilesInCurrentDirAndSubdirs) const override;

    mu::RetVal<mu::ByteArray> readFile(const mu::io::path_t& filePath) const override;

    bool readFile(const mu::io::path_t& filePath, mu::ByteArray& data) const override;
    mu::Ret writeFile(const mu::io::path_t& filePath, const mu::ByteArray& data) const override;

    void setAttribute(const mu::io::path_t& path, Attribute attribute) const override;
    bool setPermissionsAllowedForAll(const mu::io::path_t& path) const override;

    mu::io::path_t canonicalFilePath(const mu::io::path_t& filePath) const override;
    mu::io::path_t absolutePath(const mu::io::path_t& filePath) const override;
    mu::io::path_t absoluteFilePath(const mu::io::path_t& filePath) const override;

    mu::DateTime birthTime(const mu::io::path_t& filePath) const override;
    mu::DateTime lastModified(const mu::io::path_t& filePath) const override;
    bool isWritable(const mu::io::path_t& filePath) const override;

protected:

    mu::Ret removeDir(const mu::io::path_t& path) const;
    mu::Ret removeFile(const mu::io::path_t& path) const;

    bool isResourcePath(const mu::io::path_t& path) const;
    bool resourceExists(const mu::io::path_t& path) const;
    bool resourceReadFile(const mu::io::path_t& filePath, mu::ByteArray& data) const;
    mu::RetVal<mu::io::paths_t> resourceScanFiles(const mu::io::path_t& path) const;
};
}

#endif // XTZ_IO_FILESYSTEMBASE_H
