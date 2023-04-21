#ifndef XTZ_FONTS_FONTSENGINE_HPP
#define XTZ_FONTS_FONTSENGINE_HPP

#include <map>
#include <functional>

#include "../ifontsengine.hpp"

#include "global/modularity/ioc.h"
#include "fonts/ifontsdatabase.hpp"

#include "fontrendercache.hpp"

namespace xtz::fonts {
class IFontFace;
class FontsEngine : public IFontsEngine
{
    INJECT(xtz::fonts, IFontsDatabase, fontsDatabase)

public:
    FontsEngine() = default;
    ~FontsEngine();

    void init();

    double lineSpacing(const mu::draw::Font& f) const override;
    double xHeight(const mu::draw::Font& f) const override;
    double height(const mu::draw::Font& f) const override;
    double ascent(const mu::draw::Font& f) const override;
    double descent(const mu::draw::Font& f) const override;

    bool inFontUcs4(const mu::draw::Font& f, char32_t ucs4) const override;

    double horizontalAdvance(const mu::draw::Font& f, const char32_t& ch) const override;
    double horizontalAdvance(const mu::draw::Font& f, const std::u32string& text) const override;

    mu::RectF boundingRect(const mu::draw::Font& f, const char32_t& ch) const override;
    mu::RectF boundingRect(const mu::draw::Font& f, const std::u32string& text) const override;
    mu::RectF tightBoundingRect(const mu::draw::Font& f, const std::u32string& text) const override;

    // Score symbols
    mu::RectF symBBox(const mu::draw::Font& f, char32_t ucs4) const override;
    double symAdvance(const mu::draw::Font& f, char32_t ucs4) const override;

    // For draw
    std::vector<GlyphImage> render(const mu::draw::Font& f, const std::u32string& text) const override;

    // For dev
    using FontFaceFactory = std::function<IFontFace* (const mu::io::path_t&)>;
    void setFontFaceFactory(const FontFaceFactory& f);

private:

    struct TextLine {
        const char32_t* text = nullptr;
        int lenght = 0;
    };

    struct RequireFace {
        IFontFace* face = nullptr;   // real loaded face
        FaceKey requireKey;          // require face

        bool isSymbolMode() const;
        double pixelScale() const;
    };

    IFontFace* createFontFace(const mu::io::path_t& path) const;
    RequireFace* fontFace(const mu::draw::Font& f, bool isSymbolMode = false) const;

    std::vector<TextLine> splitTextByLines(const std::u32string& text) const;

    FontFaceFactory m_fontFaceFactory;

    mutable std::vector<IFontFace*> m_loadedFaces;
    mutable std::vector<RequireFace*> m_requiredFaces;

    mutable FontRenderCache m_renderCache;
};
}

#endif // XTZ_FONTS_FONTSENGINE_HPP
