#ifndef XTZ_NOTATION_SYMBOLMETRICSFM_HPP
#define XTZ_NOTATION_SYMBOLMETRICSFM_HPP

#include <vector>
#include <map>

#include "isymbolmetrics.hpp"

#include "modularity/ioc.h"
#include "draw/ifontprovider.h"

#include "draw/types/font.h"
#include "engraving/infrastructure/smufl.h"

namespace mu {
class JsonObject;
}

namespace xtz::notation {
class SymbolMetricsFM : public ISymbolMetrics
{
    INJECT_STATIC(xtz::notation, mu::draw::IFontProvider, fontProvider)
public:
    SymbolMetricsFM();

    void load(const std::string& family, const mu::io::path_t& path) override;

    const mu::draw::Font& font() const override;

    bool isValid(mu::engraving::SymId id) const override;

    char32_t symCode(mu::engraving::SymId id) const override;
    mu::engraving::SymId fromCode(char32_t code) const override;

    bool isCompound(mu::engraving::SymId id) const override;
    const mu::engraving::SymIdList& subSymbols(mu::engraving::SymId id) const override;

    const mu::RectF& bbox(mu::engraving::SymId id) const override;
    double advance(mu::engraving::SymId id) const override;

    const mu::PointF& smuflAnchor(mu::engraving::SymId symId, mu::engraving::SmuflAnchorId anchorId) const override;

    const std::unordered_map<mu::engraving::Sid, mu::engraving::PropertyValue>& engravingDefaults() const override;

private:

    struct Sym {
        char32_t code = 0;
        mu::RectF bbox;
        double advance = 0.0;

        std::map<mu::engraving::SmuflAnchorId, mu::PointF> smuflAnchors;
        mu::engraving::SymIdList subSymbolIds;

        bool isValid() const { return code != 0 && bbox.isValid(); }
        bool isCompound() const { return !subSymbolIds.empty(); }
    };

    void loadGlyphsWithAnchors(const mu::JsonObject& glyphsWithAnchors);
    void loadComposedGlyphs();
    void loadStylisticAlternates(const mu::JsonObject& glyphsWithAlternatesObject);
    void loadEngravingDefaults(const mu::JsonObject& engravingDefaultsObject);
    void computeMetrics(Sym& sym, const mu::engraving::Smufl::Code& code);

    const Sym& sym(mu::engraving::SymId id) const;
    Sym& sym(mu::engraving::SymId id);

    std::string m_family;
    mu::io::path_t m_fontPath;
    mu::draw::Font m_font;
    std::vector<Sym> m_symbols;

    std::unordered_map<mu::engraving::Sid, mu::engraving::PropertyValue> m_engravingDefaults;
};
}

#endif // XTZ_NOTATION_SYMBOLMETRICSFM_HPP
