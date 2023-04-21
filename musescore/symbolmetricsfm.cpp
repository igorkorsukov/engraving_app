#include "symbolmetricsfm.hpp"

#include "global/io/file.h"
#include "global/io/fileinfo.h"
#include "global/serialization/json.h"

#include "engraving/types/symnames.h"
#include "engraving/libmscore/mscore.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::draw;
using namespace mu::engraving;
using namespace xtz::notation;

SymbolMetricsFM::SymbolMetricsFM()
    : m_symbols(static_cast<size_t>(SymId::lastSym) + 1)
{
}

void SymbolMetricsFM::load(const std::string& family, const mu::io::path_t& path)
{
    m_family = family;
    m_fontPath = path;

    if (-1 == fontProvider()->addSymbolFont(String::fromStdString(m_family), m_fontPath)) {
        LOGE() << "fatal error: cannot load internal font: " << m_fontPath;
        return;
    }

    m_font.setWeight(mu::draw::Font::Normal);
    m_font.setItalic(false);
    m_font.setFamily(String::fromStdString(m_family), Font::Type::MusicSymbol);
    m_font.setNoFontMerging(true);
    m_font.setHinting(mu::draw::Font::Hinting::PreferVerticalHinting);

    double size = 20.0 * MScore::pixelRatio;
    m_font.setPointSizeF(size);

    for (size_t id = 0; id < m_symbols.size(); ++id) {
        Smufl::Code code = Smufl::code(static_cast<SymId>(id));
        if (!code.isValid()) {
            continue;
        }
        Sym& sym = m_symbols[id];
        computeMetrics(sym, code);
    }

    File metadataFile(FileInfo(m_fontPath).path() + u"/metadata.json");
    if (!metadataFile.open(IODevice::ReadOnly)) {
        LOGE() << "Failed to open glyph metadata file: " << metadataFile.filePath();
        return;
    }

    std::string error;
    JsonObject metadataJson = JsonDocument::fromJson(metadataFile.readAll(), &error).rootObject();
    if (!error.empty()) {
        LOGE() << "Json parse error in " << metadataFile.filePath() << ", error: " << error;
        return;
    }

    loadGlyphsWithAnchors(metadataJson.value("glyphsWithAnchors").toObject());
    loadComposedGlyphs();
    loadStylisticAlternates(metadataJson.value("glyphsWithAlternates").toObject());
    loadEngravingDefaults(metadataJson.value("engravingDefaults").toObject());
}

const mu::draw::Font& SymbolMetricsFM::font() const
{
    return m_font;
}

SymbolMetricsFM::Sym& SymbolMetricsFM::sym(SymId id)
{
    return m_symbols[static_cast<size_t>(id)];
}

const SymbolMetricsFM::Sym& SymbolMetricsFM::sym(SymId id) const
{
    return m_symbols.at(static_cast<size_t>(id));
}

char32_t SymbolMetricsFM::symCode(mu::engraving::SymId id) const
{
    const Sym& s = sym(id);
    if (s.isValid()) {
        return s.code;
    }

    // fallback: search in the common SMuFL table
    return Smufl::smuflCode(id);
}

mu::engraving::SymId SymbolMetricsFM::fromCode(char32_t code) const
{
    auto it = std::find_if(m_symbols.begin(), m_symbols.end(), [code](const Sym& s) { return s.code == code; });
    return static_cast<SymId>(it == m_symbols.end() ? 0 : it - m_symbols.begin());
}

bool SymbolMetricsFM::isValid(mu::engraving::SymId id) const
{
    return sym(id).isValid();
}

bool SymbolMetricsFM::isCompound(mu::engraving::SymId id) const
{
    return sym(id).isCompound();
}

const mu::engraving::SymIdList& SymbolMetricsFM::subSymbols(mu::engraving::SymId id) const
{
    return sym(id).subSymbolIds;
}

const mu::RectF& SymbolMetricsFM::bbox(mu::engraving::SymId id) const
{
    return sym(id).bbox;
}

double SymbolMetricsFM::advance(mu::engraving::SymId id) const
{
    return sym(id).advance;
}

const mu::PointF& SymbolMetricsFM::smuflAnchor(mu::engraving::SymId symId, mu::engraving::SmuflAnchorId anchorId) const
{
    const std::map<SmuflAnchorId, mu::PointF>& smuflAnchors = sym(symId).smuflAnchors;
    auto it = smuflAnchors.find(anchorId);
    if (it != smuflAnchors.cend()) {
        return it->second;
    }

    static const mu::PointF null;
    return null;
}

const std::unordered_map<mu::engraving::Sid, mu::engraving::PropertyValue>& SymbolMetricsFM::engravingDefaults() const
{
    return m_engravingDefaults;
}

void SymbolMetricsFM::loadGlyphsWithAnchors(const JsonObject& glyphsWithAnchors)
{
    for (const std::string& symName : glyphsWithAnchors.keys()) {
        SymId symId = SymNames::symIdByName(symName);
        if (symId == SymId::noSym) {
            //! NOTE currently, Bravura contains a bunch of entries in glyphsWithAnchors
            //! for glyph names that will not be found - flag32ndUpStraight, etc.
            continue;
        }

        Sym& sym = this->sym(symId);
        JsonObject anchors = glyphsWithAnchors.value(symName).toObject();

        static const std::unordered_map<std::string, SmuflAnchorId> smuflAnchorIdNames {
            { "stemDownNW", SmuflAnchorId::stemDownNW },
            { "stemUpSE", SmuflAnchorId::stemUpSE },
            { "stemDownSW", SmuflAnchorId::stemDownSW },
            { "stemUpNW", SmuflAnchorId::stemUpNW },
            { "cutOutNE", SmuflAnchorId::cutOutNE },
            { "cutOutNW", SmuflAnchorId::cutOutNW },
            { "cutOutSE", SmuflAnchorId::cutOutSE },
            { "cutOutSW", SmuflAnchorId::cutOutSW },
            { "opticalCenter", SmuflAnchorId::opticalCenter },
        };

        for (const std::string& anchorId : anchors.keys()) {
            auto search = smuflAnchorIdNames.find(anchorId);
            if (search == smuflAnchorIdNames.cend()) {
                //LOGD() << "Unhandled SMuFL anchorId: " << anchorId;
                continue;
            }

            JsonArray arr = anchors.value(anchorId).toArray();
            double x = arr.at(0).toDouble();
            double y = arr.at(1).toDouble();

            sym.smuflAnchors[search->second] = PointF(x, -y) * SPATIUM20;
        }
    }
}

void SymbolMetricsFM::loadComposedGlyphs()
{
    static const struct ComposedGlyph {
        const SymId id;
        const SymIdList subSymbolIds;
    } composedGlyphs[] = {
        { SymId::ornamentPrallMordent, {
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentMiddleVerticalStroke,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentUpPrall, {
              SymId::ornamentBottomLeftConcaveStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentUpMordent, {
              SymId::ornamentBottomLeftConcaveStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentMiddleVerticalStroke,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentPrallDown, {
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentBottomRightConcaveStroke,
          } },
        { SymId::ornamentDownMordent, {
              SymId::ornamentLeftVerticalStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentMiddleVerticalStroke,
              SymId::ornamentZigZagLineWithRightEnd
          } },
        { SymId::ornamentPrallUp, {
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentTopRightConvexStroke,
          } },
        { SymId::ornamentLinePrall, {
              SymId::ornamentLeftVerticalStroke,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineNoRightEnd,
              SymId::ornamentZigZagLineWithRightEnd
          } }
    };

    auto symsBbox = [this](const mu::engraving::SymIdList& sl) {
        RectF r;
        PointF pos;
        for (SymId id : sl) {
            const Sym& s = this->sym(id);
            r.unite(s.bbox.translated(pos));
            pos.rx() += s.advance;
        }
        return r;
    };

    for (const ComposedGlyph& c : composedGlyphs) {
        Sym& sym = this->sym(c.id);
        if (!sym.isValid()) {
            sym.subSymbolIds = c.subSymbolIds;
            sym.bbox = symsBbox(c.subSymbolIds);
        }
    }
}

void SymbolMetricsFM::loadStylisticAlternates(const JsonObject& glyphsWithAlternatesObject)
{
    if (!glyphsWithAlternatesObject.isValid()) {
        return;
    }

    static const struct GlyphWithAlternates {
        const std::string key;
        const std::string alternateKey;
        const SymId alternateSymId;
    } glyphsWithAlternates[] = {
        { std::string("4stringTabClef"),
          std::string("4stringTabClefSerif"),
          SymId::fourStringTabClefSerif
        },
        { std::string("6stringTabClef"),
          std::string("6stringTabClefSerif"),
          SymId::sixStringTabClefSerif
        },
        { std::string("cClef"),
          std::string("cClefFrench"),
          SymId::cClefFrench
        },
        { std::string("cClef"),
          std::string("cClefFrench20C"),
          SymId::cClefFrench20C
        },
        { std::string("fClef"),
          std::string("fClefFrench"),
          SymId::fClefFrench
        },
        { std::string("fClef"),
          std::string("fClef19thCentury"),
          SymId::fClef19thCentury
        },
        { std::string("noteheadBlack"),
          std::string("noteheadBlackOversized"),
          SymId::noteheadBlack
        },
        { std::string("noteheadHalf"),
          std::string("noteheadHalfOversized"),
          SymId::noteheadHalf
        },
        { std::string("noteheadWhole"),
          std::string("noteheadWholeOversized"),
          SymId::noteheadWhole
        },
        { std::string("noteheadDoubleWhole"),
          std::string("noteheadDoubleWholeOversized"),
          SymId::noteheadDoubleWhole
        },
        { std::string("noteheadDoubleWholeSquare"),
          std::string("noteheadDoubleWholeSquareOversized"),
          SymId::noteheadDoubleWholeSquare
        },
        { std::string("noteheadDoubleWhole"),
          std::string("noteheadDoubleWholeAlt"),
          SymId::noteheadDoubleWholeAlt
        },
        { std::string("brace"),
          std::string("braceSmall"),
          SymId::braceSmall
        },
        { std::string("brace"),
          std::string("braceLarge"),
          SymId::braceLarge
        },
        { std::string("brace"),
          std::string("braceLarger"),
          SymId::braceLarger
        },
        { std::string("flag1024thDown"),
          std::string("flag1024thDownStraight"),
          SymId::flag1024thDownStraight
        },
        { std::string("flag1024thUp"),
          std::string("flag1024thUpStraight"),
          SymId::flag1024thUpStraight
        },
        { std::string("flag128thDown"),
          std::string("flag128thDownStraight"),
          SymId::flag128thDownStraight
        },
        { std::string("flag128thUp"),
          std::string("flag128thUpStraight"),
          SymId::flag128thUpStraight
        },
        { std::string("flag16thDown"),
          std::string("flag16thDownStraight"),
          SymId::flag16thDownStraight
        },
        { std::string("flag16thUp"),
          std::string("flag16thUpStraight"),
          SymId::flag16thUpStraight
        },
        { std::string("flag256thDown"),
          std::string("flag256thDownStraight"),
          SymId::flag256thDownStraight
        },
        { std::string("flag256thUp"),
          std::string("flag256thUpStraight"),
          SymId::flag256thUpStraight
        },
        { std::string("flag32ndDown"),
          std::string("flag32ndDownStraight"),
          SymId::flag32ndDownStraight
        },
        { std::string("flag32ndUp"),
          std::string("flag32ndUpStraight"),
          SymId::flag32ndUpStraight
        },
        { std::string("flag512thDown"),
          std::string("flag512thDownStraight"),
          SymId::flag512thDownStraight
        },
        { std::string("flag512thUp"),
          std::string("flag512thUpStraight"),
          SymId::flag512thUpStraight
        },
        { std::string("flag64thDown"),
          std::string("flag64thDownStraight"),
          SymId::flag64thDownStraight
        },
        { std::string("flag64thUp"),
          std::string("flag64thUpStraight"),
          SymId::flag64thUpStraight
        },
        { std::string("flag8thDown"),
          std::string("flag8thDownStraight"),
          SymId::flag8thDownStraight
        },
        { std::string("flag8thUp"),
          std::string("flag8thUpStraight"),
          SymId::flag8thUpStraight
        }
    };

    bool ok;
    for (const GlyphWithAlternates& glyph : glyphsWithAlternates) {
        if (glyphsWithAlternatesObject.contains(glyph.key)) {
            const JsonArray alternatesArray = glyphsWithAlternatesObject.value(glyph.key).toObject().value("alternates").toArray();

            JsonValue val;
            for (size_t i = 0; i < alternatesArray.size(); ++i) {
                JsonValue v = alternatesArray.at(i);
                if (v.toObject().value("name").toStdString() == glyph.alternateKey) {
                    val = v;
                    break;
                }
            }

            if (!val.isNull()) {
                JsonObject symObj = val.toObject();
                Sym& sym = this->sym(glyph.alternateSymId);

                Smufl::Code code;
                char32_t smuflCode = symObj.value("codepoint").toString().mid(2).toUInt(&ok, 16);
                if (ok) {
                    code.smuflCode = smuflCode;
                }

                char32_t musicSymBlockCode = symObj.value("alternateCodepoint").toString().mid(2).toUInt(&ok, 16);
                if (ok) {
                    code.musicSymBlockCode = musicSymBlockCode;
                }

                if (code.smuflCode || code.musicSymBlockCode) {
                    computeMetrics(sym, code);
                }
            }
        }
    }
}

void SymbolMetricsFM::loadEngravingDefaults(const JsonObject& engravingDefaultsObject)
{
    struct EngravingDefault {
        std::vector<Sid> sids;

        // If a childKey is not specified in `engravingDefaultsObject`,
        // it will receive the value for the key of `this` EngravingDefault.
        // This is done for compatibility with fonts made for older SMuFL versions:
        // in newer versions, some settings have been split into two.
        std::vector<std::string> childKeys = {};

        EngravingDefault(const std::vector<Sid>& sids)
            : sids(sids), childKeys() {}
        EngravingDefault(const std::vector<Sid>& sids, const std::vector<std::string>& childKeys)
            : sids(sids), childKeys(childKeys) {}
    };

    // https://w3c.github.io/smufl/latest/specification/engravingdefaults.html
    static const std::unordered_map<std::string, EngravingDefault> engravingDefaultsMapping = {
        // "textFontFamily" not supported
        { "staffLineThickness",         { { Sid::staffLineWidth } } },
        { "stemThickness",              { { Sid::stemWidth } } },
        { "beamThickness",              { { Sid::beamWidth } } },
        // "beamSpacing" handled separately
        { "legerLineThickness",         { { Sid::ledgerLineWidth } } },
        { "legerLineExtension",         { { Sid::ledgerLineLength } } },
        { "slurEndpointThickness",      { { Sid::SlurEndWidth } } },
        { "slurMidpointThickness",      { { Sid::SlurMidWidth } } },
        // "tieEndpointThickness" not supported
        // "tieMidpointThickness" not supported
        { "thinBarlineThickness",       { { Sid::barWidth, Sid::doubleBarWidth } } },
        { "thickBarlineThickness",      { { Sid::endBarWidth } } },
        // "dashedBarlineThickness" not supported
        // "dashedBarlineDashLength" not supported
        // "dashedBarlineGapLength" not supported
        { "barlineSeparation",          { { Sid::doubleBarDistance }, { "thinThickBarlineSeparation" } } },
        { "thinThickBarlineSeparation", { { Sid::endBarDistance } } },
        { "repeatBarlineDotSeparation", { { Sid::repeatBarlineDotSeparation } } },
        { "bracketThickness",           { { Sid::bracketWidth } } },
        // "subBracketThickness" not supported
        { "hairpinThickness",           { { Sid::hairpinLineWidth } } },
        { "octaveLineThickness",        { { Sid::ottavaLineWidth } } },
        { "pedalLineThickness",         { { Sid::pedalLineWidth } } },
        { "repeatEndingLineThickness",  { { Sid::voltaLineWidth } } },
        // "arrowShaftThickness" not supported
        { "lyricLineThickness",         { { Sid::lyricsLineThickness } } },
        // "textEnclosureThickness" handled separately
        { "tupletBracketThickness",     { { Sid::tupletBracketWidth } } },
        { "hBarThickness",              { { Sid::mmRestHBarThickness } } }
    };

    std::function<void(const std::string& key, const PropertyValue& value)> applyEngravingDefault;

    applyEngravingDefault = [&](const std::string& key, const PropertyValue& value) {
        auto search = engravingDefaultsMapping.find(key);
        if (search != engravingDefaultsMapping.cend()) {
            for (Sid sid : search->second.sids) {
                m_engravingDefaults.insert({ sid, value });
            }

            for (const std::string& childKey : search->second.childKeys) {
                if (!engravingDefaultsObject.contains(childKey)) {
                    applyEngravingDefault(childKey, value);
                }
            }
        }
    };

    for (const std::string& key : engravingDefaultsObject.keys()) {
        if (key == "textEnclosureThickness") {
            continue;
        }

        if (key == "beamSpacing") {
            bool value = engravingDefaultsObject.value(key).toDouble() > 0.75;
            m_engravingDefaults.insert({ Sid::useWideBeams, value });
            continue;
        }

        applyEngravingDefault(key, engravingDefaultsObject.value(key).toDouble());
    }

    m_engravingDefaults.insert({ Sid::MusicalTextFont, String(u"%1 Text").arg(String::fromStdString(m_family)) });
}

void SymbolMetricsFM::computeMetrics(Sym& sym, const Smufl::Code& code)
{
    if (fontProvider()->inFontUcs4(m_font, code.smuflCode)) {
        sym.code = code.smuflCode;
    } else if (fontProvider()->inFontUcs4(m_font, code.musicSymBlockCode)) {
        sym.code = code.musicSymBlockCode;
    }

    if (sym.code > 0) {
        sym.bbox = fontProvider()->symBBox(m_font, sym.code, DPI_F);
        sym.advance = fontProvider()->symAdvance(m_font, sym.code, DPI_F);
    }
}
