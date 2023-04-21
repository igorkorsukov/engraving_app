#ifndef XTZ_FONTS_FONTFACEXT_H
#define XTZ_FONTS_FONTFACEXT_H

#include <unordered_map>

// mu
#include "global/io/iodevice.h"

// xtz
#include "ifontface.hpp"

namespace mu {
class ZipReader;
}

namespace xtz::fonts {
class FontFaceXT : public IFontFace
{
public:
    FontFaceXT();
    ~FontFaceXT();

    struct GlyphData {
        GlyphData() = default;
        GlyphData(GlyphData&& o) = default;
        FBBox textBbox;
        f26dot6_t textAdvance = 0;
        FBBox symBbox;
        f26dot6_t symAdvance = 0;
        msdfgen::Shape shape;

        void write(mu::io::IODevice* d) const;
        void read(mu::io::IODevice* d);
    };

    bool load(const FaceKey& key, const mu::io::path_t& path, bool isSymbolMode) override;

    const FaceKey& key() const override;
    bool isSymbolMode() const override;

    f26dot6_t leading() const override;
    f26dot6_t ascent() const override;
    f26dot6_t descent() const override;
    f26dot6_t xHeight() const override;

    std::vector<GlyphPos> glyphs(const char32_t* text, int text_length) const override;
    glyph_idx_t glyphIndex(char32_t ucs4) const override;
    char32_t findCharCode(glyph_idx_t idx) const override;

    FBBox glyphBbox(glyph_idx_t idx) const override;
    f26dot6_t glyphAdvance(glyph_idx_t idx) const override;
    const msdfgen::Shape& glyphShape(glyph_idx_t idx) const override;

    const std::set<char32_t>& chars() const;

    using Ligature = std::pair<char32_t, std::vector<char32_t> >;
    using Ligatures = std::vector<Ligature>;

    static void applyLigatures(std::vector<char32_t>& text, const Ligatures& ls);

private:

    const GlyphData& glyphData(glyph_idx_t idx) const;

    FaceKey m_key;
    bool m_isSymbolMode = false;

    mu::ZipReader* m_zip = nullptr;
    f26dot6_t m_leading = -1;
    f26dot6_t m_ascent = -1;
    f26dot6_t m_descent = -1;
    f26dot6_t m_xHeight = -1;

    Ligatures m_ligatures;

    mutable std::set<char32_t> m_chars;
    mutable std::unordered_map<glyph_idx_t, GlyphData> m_cache;
};
}

#endif // XTZ_FONTS_FONTFACEXT_H
