#ifndef XTZ_FONTS_FONTSTYPES_HPP
#define XTZ_FONTS_FONTSTYPES_HPP

#include <string>
#include <vector>

// mu
#include "global/types/bytearray.h"
#include "global/stringutils.h"
#include "draw/types/font.h"
#include "draw/types/geometry.h"

namespace xtz::fonts {
using glyph_idx_t = uint32_t;

static constexpr double DPI_F = 5.0;
static constexpr double DPI = 72.0 * DPI_F;

struct FontDataKey {
public:

    FontDataKey() = default;
    FontDataKey(const std::string& fa)
        : m_family(mu::strings::toLower(fa)), m_bold(false), m_italic(false) {}

    FontDataKey(const std::string& fa, bool bo, bool it)
        : m_family(mu::strings::toLower(fa)), m_bold(bo), m_italic(it) {}

    inline bool valid() const { return !m_family.empty(); }

    const std::string& family() const { return m_family; }
    bool bold() const { return m_bold; }
    bool italic() const { return m_italic; }

    inline bool operator==(const FontDataKey& o) const
    {
        return m_bold == o.m_bold && m_italic == o.m_italic && m_family == o.m_family;
    }

    inline bool operator!=(const FontDataKey& o) const { return !this->operator==(o); }

    inline bool operator<(const FontDataKey& o) const
    {
        if (m_bold != o.m_bold) {
            return m_bold < o.m_bold;
        }

        if (m_italic != o.m_italic) {
            return m_italic < o.m_italic;
        }

        return m_family < o.m_family;
    }

private:
    std::string m_family;
    bool m_bold = false;
    bool m_italic = false;
};

inline FontDataKey dataKeyForFont(const mu::draw::Font& f)
{
    return FontDataKey(f.family().toStdString(), f.bold(), f.italic());
}

struct FontData {
    FontDataKey key;
    mu::ByteArray data;

    inline bool valid() const { return key.valid() && !data.empty(); }
};

struct FaceKey {
    FontDataKey dataKey;
    mu::draw::Font::Type type = mu::draw::Font::Type::Undefined;
    int pixelSize = 0;

    FaceKey() = default;

    inline bool operator==(const FaceKey& o) const
    {
        return type == o.type && dataKey == o.dataKey && pixelSize == o.pixelSize;
    }

    inline bool operator!=(const FaceKey& o) const { return !this->operator==(o); }
    inline bool operator<(const FaceKey& o) const
    {
        if (type != o.type) {
            return type < o.type;
        } else if (dataKey != o.dataKey) {
            return dataKey < o.dataKey;
        } else {
            return pixelSize < o.pixelSize;
        }
    }
};

inline int pixelSizeForFont(const mu::draw::Font& f)
{
    if (f.pixelSize() > 0) {
        return f.pixelSize();
    } else {
        return f.pointSizeF() * DPI / 72.0;
    }
}

inline FaceKey faceKeyForFont(const mu::draw::Font& f)
{
    return FaceKey{ dataKeyForFont(f), f.type(), pixelSizeForFont(f) };
}

struct Sdf {
    mu::ByteArray bitmap;
    uint32_t width = 0;
    uint32_t height = 0;
    float threshold = 0.;
};

struct GlyphImage {
    mu::RectF rect;
    Sdf sdf;

    bool isNull() const { return rect.isNull(); }
};

struct FontParams {
    std::string name;
    mu::draw::Font::Type type = mu::draw::Font::Type::Undefined;
    bool bold{ false };
    bool italic{ false };
    float pixelSize{ 0. };
};
}

#endif // XTZ_FONTS_FONTSTYPES_HPP
