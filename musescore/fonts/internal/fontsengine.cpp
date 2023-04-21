#include "fontsengine.hpp"

#include <msdfgen.h>
#include <ext/import-font.h>

// mu
#include "global/io/fileinfo.h"

// xtz
#include "ifontface.hpp"
#include "fontfaceft.hpp"
#include "fontfacext.hpp"
#include "fontfacedu.hpp"

#include "log.h"

using namespace xtz::fonts;

static const double DEFAULT_PIXEL_SIZE = 100.0;
static const double SYMBOLS_PIXEL_SIZE = 200.0;
static const double LOADED_PIXEL_SIZE = 200.0;

static const double TEXT_LINE_SCALE = 1.2;

static const int SDF_WIDTH = 64;
static const int SDF_HEIGHT = 64;

static inline mu::RectF fromFBBox(const FBBox& bb, double scale)
{
    return mu::RectF(from_f26d6(bb.left()) * scale, from_f26d6(bb.top()) * scale,
                     from_f26d6(bb.width()) * scale, from_f26d6(bb.height()) * scale);
}

static inline mu::RectF scaleRect(const mu::RectF& r, double scale)
{
    return mu::RectF(r.x() * scale, r.y() * scale, r.width() * scale, r.height() * scale);
}

bool FontsEngine::RequireFace::isSymbolMode() const
{
    return face ? face->isSymbolMode() : false;
}

double FontsEngine::RequireFace::pixelScale() const
{
    if (!face) {
        return 0.0;
    }
    double scale = static_cast<double>(requireKey.pixelSize) / static_cast<double>(face->key().pixelSize);
    return scale;
}

FontsEngine::~FontsEngine()
{
    for (RequireFace* f : m_requiredFaces) {
        delete f;
    }

    for (IFontFace* f : m_loadedFaces) {
        delete f;
    }
}

void FontsEngine::init()
{
    m_renderCache.init();
}

double FontsEngine::lineSpacing(const mu::draw::Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->leading() + rf->face->ascent() + rf->face->descent()) * rf->pixelScale();
}

double FontsEngine::xHeight(const mu::draw::Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->xHeight()) * rf->pixelScale();
}

double FontsEngine::height(const mu::draw::Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->ascent() + rf->face->descent()) * rf->pixelScale();
}

double FontsEngine::ascent(const mu::draw::Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->ascent()) * rf->pixelScale();
}

double FontsEngine::descent(const mu::draw::Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->descent()) * rf->pixelScale();
}

bool FontsEngine::inFontUcs4(const mu::draw::Font& f, char32_t ucs4) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return false;
    }

    return rf->face->glyphIndex(ucs4) != 0;
}

double FontsEngine::horizontalAdvance(const mu::draw::Font& f, const char32_t& ch) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    glyph_idx_t glyphIdx = rf->face->glyphIndex(ch);
    return from_f26d6(rf->face->glyphAdvance(glyphIdx)) * rf->pixelScale();
}

double FontsEngine::horizontalAdvance(const mu::draw::Font& f, const std::u32string& text) const
{
    if (text.empty()) {
        return 0.0;
    }

    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    std::vector<GlyphPos> glyphs = rf->face->glyphs(&text[0], (int)text.size());
    f26dot6_t advance = 0;
    for (const GlyphPos& g : glyphs) {
        advance += g.x_advance;
    }

    return from_f26d6(advance) * rf->pixelScale();
}

mu::RectF FontsEngine::boundingRect(const mu::draw::Font& f, const char32_t& ch) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return mu::RectF();
    }

    glyph_idx_t glyphIdx = rf->face->glyphIndex(ch);
    return fromFBBox(rf->face->glyphBbox(glyphIdx), rf->pixelScale());
}

mu::RectF FontsEngine::boundingRect(const mu::draw::Font& f, const std::u32string& text) const
{
    if (text.empty()) {
        return mu::RectF();
    }

    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return mu::RectF();
    }

    FBBox rect;      // f26dot6_t units
    FBBox lineRect;  // f26dot6_t units
    bool isFirstLine = true;
    bool isFirstInLine = true;

    std::vector<TextLine> lines = splitTextByLines(text);
    for (const TextLine& l : lines) {
        lineRect = FBBox();
        isFirstInLine = true;

        std::vector<GlyphPos> glyphs = rf->face->glyphs(l.text, l.lenght);
        for (const GlyphPos& g : glyphs) {
            FBBox bbox = rf->face->glyphBbox(g.idx);
            if (isFirstInLine) {
                lineRect = bbox;
                isFirstInLine = false;
            } else {
                lineRect.setWidth(lineRect.width() + bbox.width());
                lineRect.setHeight(std::max(lineRect.height(), bbox.height()));
                lineRect.setTop(std::min(lineRect.top(), bbox.top()));
                lineRect.setLeft(std::min(lineRect.left(), bbox.left()));
            }
        }

        if (isFirstLine) {
            rect = lineRect;
            isFirstLine = false;
        } else {
            rect.setWidth(std::max(rect.width(), lineRect.width()));
            rect.setHeight(rect.height() + lineRect.height());
        }
    }

    return fromFBBox(rect, rf->pixelScale());
}

mu::RectF FontsEngine::tightBoundingRect(const mu::draw::Font& f, const std::u32string& text) const
{
    if (text.empty()) {
        return mu::RectF();
    }

    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return mu::RectF();
    }

    FBBox rect;      // f26dot6_t units
    FBBox lineRect;  // f26dot6_t units
    bool isFirstLine = true;
    bool isFirstInLine = true;

    std::vector<TextLine> lines = splitTextByLines(text);
    for (const TextLine& l : lines) {
        lineRect = FBBox();
        isFirstInLine = true;

        std::vector<GlyphPos> glyphs = rf->face->glyphs(l.text, l.lenght);
        for (const GlyphPos& g : glyphs) {
            FBBox bbox = rf->face->glyphBbox(g.idx);

            if (isFirstInLine) {
                lineRect = bbox;
                isFirstInLine = false;
            } else {
                /// width is calculated as x_advance instead
                lineRect.setHeight(std::max(lineRect.height(), bbox.height()));
                lineRect.setTop(std::min(lineRect.top(), bbox.top()));
                lineRect.setLeft(std::min(lineRect.left(), bbox.left()));
            }
        }

        f26dot6_t advance = 0;
        for (const GlyphPos& g : glyphs) {
            advance += g.x_advance;
        }

        GlyphPos lastGlyph = glyphs.back();
        advance -= (lastGlyph.x_advance - rf->face->glyphBbox(lastGlyph.idx).width());
        lineRect.setWidth(advance);

        if (isFirstLine) {
            rect = lineRect;
            isFirstLine = false;
        } else {
            rect.setWidth(std::max(rect.width(), lineRect.width()));
            rect.setHeight(rect.height() + lineRect.height());
        }
    }

    return fromFBBox(rect, rf->pixelScale());
}

mu::RectF FontsEngine::symBBox(const mu::draw::Font& f, char32_t ucs4) const
{
    RequireFace* rf = fontFace(f, true);
    IF_ASSERT_FAILED(rf && rf->face) {
        return mu::RectF();
    }

    glyph_idx_t glyphIdx = rf->face->glyphIndex(ucs4);
    FBBox bb = rf->face->glyphBbox(glyphIdx);
    return fromFBBox(bb, rf->pixelScale());
}

double FontsEngine::symAdvance(const mu::draw::Font& f, char32_t ucs4) const
{
    RequireFace* rf = fontFace(f, true);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    glyph_idx_t glyphIdx = rf->face->glyphIndex(ucs4);
    f26dot6_t advance = rf->face->glyphAdvance(glyphIdx);
    return from_f26d6(advance) * rf->pixelScale();
}

static void generateSdf(GlyphImage& out, glyph_idx_t glyphIdx, IFontFace* face)
{
    struct Bounds
    {
        double l, b, r, t;
    };
    Bounds bounds = { 1e240, 1e240, -1e240, -1e240 };

    msdfgen::Shape shape = face->glyphShape(glyphIdx);
    if (shape.contours.empty()) {
        //! NOTE Maybe not printable, like ' '
        return;
    }

    shape.bounds(bounds.l, bounds.b, bounds.r, bounds.t);

    uint32_t pxRange = std::min(SDF_WIDTH, SDF_HEIGHT) >> 3;

    std::pair<double, double> sdfScale;
    msdfgen::Vector2 translate;
    double scale = 0.0;
    msdfgen::Vector2 frame(SDF_WIDTH, SDF_HEIGHT);
    frame -= 2 * pxRange;
    assert(frame.x >= 0 && frame.y >= 0 && bounds.l < bounds.r && bounds.b < bounds.t);
    msdfgen::Vector2 dims(bounds.r - bounds.l, bounds.t - bounds.b);
    if (dims.x * frame.y < dims.y * frame.x) { // fit restricted by height
        translate = { -bounds.l, -bounds.b };
        scale = frame.y / dims.y;
        sdfScale = { (frame.x - dims.x * scale) / (dims.x * scale), 0.0f };
    } else { // fit restricted by width
        translate = { -bounds.l, -bounds.b };
        scale = frame.x / dims.x;
        sdfScale = { 0.0, (frame.y - dims.y * scale) / (dims.y * scale) };
    }

    double boundsWidth = bounds.r - bounds.l;
    double boundsHeight = bounds.t - bounds.b;
    double widthWhitespace = boundsWidth * sdfScale.first;
    double heightWhitespace = boundsHeight * sdfScale.second;
    double pxRangeScaled = pxRange / scale;

    double left = bounds.l - pxRangeScaled;
    double top = -bounds.t - heightWhitespace - pxRangeScaled;
    double width = boundsWidth + widthWhitespace + pxRangeScaled * 2;
    double height = boundsHeight + heightWhitespace + pxRangeScaled * 2;

    double range = pxRange / scale;
    translate += range;

    shape.mergeContours();

    auto sdf = msdfgen::Bitmap<uint8_t>(SDF_WIDTH, SDF_HEIGHT);
    msdfgen::generateSDF(sdf, shape, bounds.l, range, scale, translate);

    out.sdf.bitmap = mu::ByteArray(sdf.takeMemoryAway(), SDF_WIDTH * SDF_HEIGHT);
    out.sdf.width = SDF_WIDTH;
    out.sdf.height = SDF_HEIGHT;

    out.rect.setTop(top);
    out.rect.setLeft(left);
    out.rect.setWidth(width);
    out.rect.setHeight(height);
}

std::vector<GlyphImage> FontsEngine::render(const mu::draw::Font& f, const std::u32string& text) const
{
    //! NOTE for rendering, all fonts, including symbols fonts, are processed as text
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return std::vector<GlyphImage>();
    }

    static const std::set<glyph_idx_t> NOT_RENDER_GLYPHS = {
        3 // space
    };

    std::vector<GlyphImage> images;

    int pixelSize = rf->requireKey.pixelSize;
    double pixelScale = rf->pixelScale();
    double glyphTop = 0;
    std::vector<TextLine> lines = splitTextByLines(text);

    for (const TextLine& l : lines) {
        std::vector<GlyphPos> glyphs = rf->face->glyphs(l.text, l.lenght);

        double glyphLeft = 0;
        for (const GlyphPos& g : glyphs) {
            if (NOT_RENDER_GLYPHS.find(g.idx) == NOT_RENDER_GLYPHS.end()) {
                GlyphImage image = m_renderCache.load(rf->face->key(), g.idx);
                if (image.isNull()) {
                    generateSdf(image, g.idx, rf->face);
                    m_renderCache.store(rf->face->key(), g.idx, image);
                }

                image.rect = scaleRect(image.rect, pixelScale);
                image.rect.translate(glyphLeft, glyphTop);

                images.push_back(std::move(image));
            }

            glyphLeft += from_f26d6(g.x_advance) * pixelScale;
        }

        glyphTop += (pixelSize * TEXT_LINE_SCALE);
    }

    return images;
}

void FontsEngine::setFontFaceFactory(const FontFaceFactory& f)
{
    m_fontFaceFactory = f;
}

IFontFace* FontsEngine::createFontFace(const mu::io::path_t& path) const
{
    if (m_fontFaceFactory) {
        return m_fontFaceFactory(path);
    }

    IFontFace* origin = nullptr;
    if (mu::io::FileInfo::suffix(path) == u"ftx") {
        origin = new FontFaceXT();
    } else {
        origin = new FontFaceFT();
    }

    return new FontFaceDU(origin);
}

FontsEngine::RequireFace* FontsEngine::fontFace(const mu::draw::Font& f, bool isSymbolMode) const
{
    //! NOTE This font is required
    FaceKey requireKey = faceKeyForFont(f);

    //! NOTE If pixelSize is not set, then specify the default
    //! (this is the default pixelSize in Qt)
    if (!(requireKey.pixelSize > 0)) {
        requireKey.pixelSize = DEFAULT_PIXEL_SIZE;
    }

    //! NOTE For symbol mode, a fixed pixelSize is used
    if (isSymbolMode) {
        requireKey.pixelSize = SYMBOLS_PIXEL_SIZE;
    }

    //! NOTE At the moment, in some cases, the type may not be specified,
    //! so set as Text
    if (requireKey.type == mu::draw::Font::Type::Undefined || requireKey.type == mu::draw::Font::Type::Unknown) {
        requireKey.type = mu::draw::Font::Type::Text;
    }

    //! NOTE We are looking for the require font we need among the previously loaded ones
    for (RequireFace* face : m_requiredFaces) {
        if (face->requireKey == requireKey && face->isSymbolMode() == isSymbolMode) {
            return face;
        }
    }

    //! If we didn't find it, we create a new require font
    RequireFace* newFont = new RequireFace();
    newFont->requireKey = requireKey;

    //! Let's find out which real font will be used
    //! (for example, if there is no required one)
    FontDataKey actualDataKey = fontsDatabase()->actualFont(requireKey.dataKey, requireKey.type);

    //! NOTE We are looking for the font face we real need among the previously loaded ones
    //! IMPORTANT We use font faces with a fixed pixelSize, so we need to find the right face only from the data
    IFontFace* face = nullptr;
    for (IFontFace* f : m_loadedFaces) {
        if (f->key().dataKey == actualDataKey && f->isSymbolMode() == isSymbolMode) {
            face = f;
            break;
        }
    }

    //! NOTE If we haven't found a face, we'll create a new one
    if (!face) {
        mu::io::path_t fontPath = fontsDatabase()->fontPath(requireKey.dataKey, requireKey.type);
        IF_ASSERT_FAILED(!fontPath.empty()) {
            return nullptr;
        }

        FaceKey loadedKey;
        loadedKey.dataKey = actualDataKey;
        loadedKey.type = requireKey.type;
        loadedKey.pixelSize = LOADED_PIXEL_SIZE;

        face = createFontFace(fontPath);

        face->load(loadedKey, fontPath, isSymbolMode);
        m_loadedFaces.push_back(face);
    }

    newFont->face = face;
    m_requiredFaces.push_back(newFont);

    return newFont;
}

std::vector<FontsEngine::TextLine> FontsEngine::splitTextByLines(const std::u32string& text) const
{
    std::vector<TextLine> lines;

    TextLine l;
    for (size_t i = 0; i < text.size(); ++i) {
        if (!l.text) {
            l.text = &text[i];
        }
        ++l.lenght;

        if (i == (text.size() - 1) || text.at(i) == U'\n') {
            lines.push_back(l);
            l.text = nullptr;
            l.lenght = 0;
        }
    }

    return lines;
}
