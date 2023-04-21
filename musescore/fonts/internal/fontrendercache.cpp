#include "fontrendercache.hpp"

#include <string>
#include <iomanip>

#include "global/stringutils.h"
#include "global/io/file.h"
#include "global/io/fileinfo.h"
#include "global/io/dir.h"

//#include "xtz_global/io/io.hpp"
//#include "xtz_global/version.h"

#include "log.h"

using namespace xtz::fonts;

void FontRenderCache::init()
{
    // clean cache if need
    if (isStoreToFS()) {
        const mu::io::path_t cachePath = cacheDirPath();
        const mu::io::path_t revisionPath = cachePath + "revision";
        bool isNeedClear = true;

        // try read revision
        if (mu::io::File::exists(revisionPath)) {
            mu::io::File file(revisionPath);
            file.open(mu::io::IODevice::ReadOnly);
            mu::ByteArray data = file.readAll();
            if (std::strcmp(data.constChar(), "xtz::Version::revision()") == 0) {
                isNeedClear = false;
            }
        }

        // clean if need
        if (isNeedClear) {
            mu::RetVal<mu::io::paths_t> files = mu::io::Dir::scanFiles(cachePath, {}, mu::io::ScanMode::FilesInCurrentDir);
            for (const mu::io::path_t& p : files.val) {
                mu::io::File::remove(p);
            }
        }

        mu::io::Dir::mkpath(cachePath);

        // write revision
        {
            mu::io::File file(revisionPath);
            file.open(mu::io::IODevice::WriteOnly);
            mu::ByteArray data = "xtz::Version::revision()";
            file.write(data);
        }
    }
}

bool FontRenderCache::isStoreToFS() const
{
#ifdef PLATFORM_WEB
    return false;
#else
    return true;
#endif
}

const mu::io::path_t& FontRenderCache::resDirPath() const
{
    static mu::io::path_t path = ":/SDFCache/";
    return path;
}

const mu::io::path_t& FontRenderCache::cacheDirPath() const
{
    static mu::io::path_t path = "/SDFCache/"; //xtz::io::CachePath() + "/SDFCache/";
    return path;
}

static std::string realToString(double n)
{
    return std::to_string(int(n * 100));
}

static double realFromString(const std::string& str)
{
    return std::stod(str) / 100.0;
}

static std::string keyToString(const FaceKey& face, glyph_idx_t glyphIdx)
{
    std::string str;
    str.reserve(50);
    str += face.dataKey.family();
    str += "_" + std::to_string(glyphIdx);
    str += "_" + std::to_string(face.dataKey.bold());
    str += "_" + std::to_string(face.dataKey.italic());
    str += "_" + std::to_string(face.pixelSize);
    return str;
}

mu::io::path_t FontRenderCache::makeFilePath(const FaceKey& face, glyph_idx_t glyphIdx, const GlyphImage& image) const
{
    std::string str;
    str.reserve(100);
    str += keyToString(face, glyphIdx);
    str += "_[" + std::to_string(image.sdf.width) + "|" + std::to_string(image.sdf.height) + "|"
           + realToString(image.rect.x()) + "|" + realToString(image.rect.y()) + "|"
           + realToString(image.rect.width()) + "|" + realToString(image.rect.height());
    str += "].sdf";

    return mu::io::path_t(str);
}

void FontRenderCache::store(const FaceKey& face, glyph_idx_t glyphIdx, const GlyphImage& image)
{
    GlyphImages& images = m_cache[face];
#ifdef DEBUG
    if (images.find(glyphIdx) != images.end()) {
        assert(images.find(glyphIdx) == images.end());
    }
#endif
    images[glyphIdx] = image;

    if (isStoreToFS()) {
        mu::io::path_t fileFullPath = cacheDirPath() + makeFilePath(face, glyphIdx, image);
        mu::io::File file(fileFullPath);
        if (!file.open(mu::io::IODevice::WriteOnly)) {
            LOGE() << "failed open to write file: " << fileFullPath;
            return;
        }

        file.write(image.sdf.bitmap);
    }
}

GlyphImage FontRenderCache::load(const FaceKey& face, glyph_idx_t glyphIdx) const
{
    auto fit = m_cache.find(face);
    if (fit != m_cache.end()) {
        const GlyphImages& images = fit->second;

        auto iit = images.find(glyphIdx);
        if (iit != images.end()) {
            return iit->second;
        }
    }

    if (m_cacheInfoMap.empty()) {
        loadCachedInfo(m_cacheInfoMap, resDirPath());
        if (isStoreToFS()) {
            loadCachedInfo(m_cacheInfoMap, cacheDirPath());
        }
    }

    CacheInfo info;
    auto it = m_cacheInfoMap.find(keyToString(face, glyphIdx));
    if (it != m_cacheInfoMap.end()) {
        info = it->second;
    }

    if (!info.filePath.empty()) {
        mu::io::File file(info.filePath);
        if (!file.open(mu::io::IODevice::ReadOnly)) {
            LOGE() << "failed read file: " << file.filePath();
            return GlyphImage();
        }

        GlyphImage image;
        image.sdf.bitmap = file.readAll();
        image.sdf.width = info.width;
        image.sdf.height = info.height;
        image.rect = info.rect;

        GlyphImages& images = m_cache[face];
#ifdef DEBUG
        if (images.find(glyphIdx) != images.end()) {
            assert(images.find(glyphIdx) == images.end());
        }
#endif
        images[glyphIdx] = image;

        return image;
    }

    return GlyphImage();
}

void FontRenderCache::loadCachedInfo(CacheInfoMap& map, const mu::io::path_t& dir) const
{
    mu::RetVal<mu::io::paths_t> files = mu::io::Dir::scanFiles(dir, {}, mu::io::ScanMode::FilesInCurrentDir);
    for (const mu::io::path_t& p : files.val) {
        std::string str = mu::io::FileInfo(p).baseName().toStdString();

        size_t startInfoIdx = str.find('[');
        if (startInfoIdx == std::string::npos) {
            continue;
        }

        size_t endInfoIdx = str.find(']');
        if (endInfoIdx == std::string::npos) {
            continue;
        }

        std::string key = str.substr(0, startInfoIdx - 1);
        std::string data = str.substr(startInfoIdx + 1, endInfoIdx - startInfoIdx - 1);
        std::vector<std::string> params;
        mu::strings::split(data, params, "|");
        if (params.size() != 6) {
            continue;
        }

        CacheInfo info;
        info.filePath = p;
        info.width = std::stoi(params.at(0));
        info.height = std::stoi(params.at(1));
        info.rect = mu::RectF(realFromString(params.at(2)), realFromString(params.at(3)),
                              realFromString(params.at(4)), realFromString(params.at(5)));

        map[key] = info;
    }
}
