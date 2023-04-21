#ifndef XTZ_FONTS_FONTFACEFT_HPP
#define XTZ_FONTS_FONTFACEFT_HPP

// xtz
#include "ifontface.hpp"

namespace xtz::fonts {
struct FData;
struct GlyphMetrics;
struct SymbolMetrics;
class FontFaceFT : public IFontFace
{
public:

    FontFaceFT();
    ~FontFaceFT();

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

private:

    GlyphMetrics* glyphMetrics(glyph_idx_t idx) const;
    SymbolMetrics* symbolMetrics(glyph_idx_t idx) const;

    FaceKey m_key;
    bool m_isSymbolMode = false;
    FData* m_data = nullptr;
    mutable std::unordered_map<glyph_idx_t, msdfgen::Shape> m_cache;
};
}

#endif // XTZ_FONTS_FONTFACEFT_HPP
