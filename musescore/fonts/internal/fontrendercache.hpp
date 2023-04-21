#ifndef XTZ_FONTS_FONTRENDERCACHE_H
#define XTZ_FONTS_FONTRENDERCACHE_H

#include <map>

#include "global/io/path.h"

#include "fonts/fontstypes.hpp"

namespace xtz::fonts {
class FontRenderCache
{
public:
    FontRenderCache() = default;

    void init();

    void store(const FaceKey& face, glyph_idx_t glyphIdx, const GlyphImage& image);
    GlyphImage load(const FaceKey& face, glyph_idx_t glyphIdx) const;

private:

    bool isStoreToFS() const;

    const mu::io::path_t& resDirPath() const;
    const mu::io::path_t& cacheDirPath() const;
    mu::io::path_t makeFilePath(const FaceKey& face, glyph_idx_t glyphIdx, const GlyphImage& image) const;

    using GlyphImages = std::unordered_map<glyph_idx_t, GlyphImage>;

    mutable std::map<FaceKey, GlyphImages> m_cache;

    struct CacheInfo {
        mu::io::path_t filePath;
        uint32_t width = 0;
        uint32_t height = 0;
        mu::RectF rect;
    };

    using CacheInfoMap = std::map<std::string /*key*/, CacheInfo>;

    void loadCachedInfo(CacheInfoMap& map, const mu::io::path_t& dir) const;

    mutable CacheInfoMap m_cacheInfoMap;
};
}

#endif // XTZ_FONTS_FONTRENDERCACHE_H
