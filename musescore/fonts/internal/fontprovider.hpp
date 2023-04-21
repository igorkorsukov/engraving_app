#ifndef XTZ_FONTS_FONTPROVIDER_H
#define XTZ_FONTS_FONTPROVIDER_H

// mu
#include "framework/draw/ifontprovider.h"
#include "global/modularity/ioc.h"

// xtz
#include "fonts/ifontsdatabase.hpp"
#include "fonts/ifontsengine.hpp"

namespace xtz::fonts {
class FontProvider : public mu::draw::IFontProvider
{
    INJECT(xtz::fonts, IFontsDatabase, fontsDatabase)
    INJECT(xtz::fonts, IFontsEngine, fontsEngine)

public:
    FontProvider() = default;

    int addSymbolFont(const mu::String& family, const mu::io::path_t& path) override;
    int addTextFont(const mu::io::path_t& path) override;
    void insertSubstitution(const mu::String& familyName, const mu::String& to) override;

    double lineSpacing(const mu::draw::Font& f) const override;
    double xHeight(const mu::draw::Font& f) const override;
    double height(const mu::draw::Font& f) const override;
    double ascent(const mu::draw::Font& f) const override;
    double descent(const mu::draw::Font& f) const override;

    bool inFont(const mu::draw::Font& f, mu::Char ch) const override;
    bool inFontUcs4(const mu::draw::Font& f, char32_t ucs4) const override;

    // Text
    double horizontalAdvance(const mu::draw::Font& f, const mu::String& string) const override;
    double horizontalAdvance(const mu::draw::Font& f, const mu::Char& ch) const override;

    mu::RectF boundingRect(const mu::draw::Font& f, const mu::String& string) const override;
    mu::RectF boundingRect(const mu::draw::Font& f, const mu::Char& ch) const override;
    mu::RectF boundingRect(const mu::draw::Font& f, const mu::RectF& r, int flags, const mu::String& string) const override;
    mu::RectF tightBoundingRect(const mu::draw::Font& f, const mu::String& string) const override;

    // Score symbols
    mu::RectF symBBox(const mu::draw::Font& f, char32_t ucs4, double DPI_F) const override;
    double symAdvance(const mu::draw::Font& f, char32_t ucs4, double DPI_F) const override;
};
}

#endif // XTZ_FONTS_FONTPROVIDER_H
