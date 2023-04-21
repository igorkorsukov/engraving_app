#ifndef XTZ_FONTS_IFONTSENGINE_HPP
#define XTZ_FONTS_IFONTSENGINE_HPP

#include <string>

// mu
#include "global/modularity/imoduleexport.h"
#include "draw/types/font.h"
#include "draw/types/geometry.h"

// xtz
#include "fontstypes.hpp"

namespace xtz::fonts {
class IFontsEngine : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(xtz::fonts::IFontsEngine)
public:
    virtual ~IFontsEngine() = default;

    virtual double lineSpacing(const mu::draw::Font& f) const = 0;
    virtual double xHeight(const mu::draw::Font& f) const = 0;
    virtual double height(const mu::draw::Font& f) const = 0;
    virtual double ascent(const mu::draw::Font& f) const = 0;
    virtual double descent(const mu::draw::Font& f) const = 0;

    virtual bool inFontUcs4(const mu::draw::Font& f, char32_t ucs4) const = 0;

    virtual double horizontalAdvance(const mu::draw::Font& f, const char32_t& ch) const = 0;
    virtual double horizontalAdvance(const mu::draw::Font& f, const std::u32string& text) const = 0;

    virtual mu::RectF boundingRect(const mu::draw::Font& f, const char32_t& ch) const = 0;
    virtual mu::RectF boundingRect(const mu::draw::Font& f, const std::u32string& text) const = 0;
    virtual mu::RectF tightBoundingRect(const mu::draw::Font& f, const std::u32string& text) const = 0;

    // Score symbols
    virtual mu::RectF symBBox(const mu::draw::Font& f, char32_t ucs4) const = 0;
    virtual double symAdvance(const mu::draw::Font& f, char32_t ucs4) const = 0;

    // Draw
    virtual std::vector<GlyphImage> render(const mu::draw::Font& f, const std::u32string& text) const = 0;
};
}

#endif // XTZ_FONTS_IFONTSENGINE_HPP
