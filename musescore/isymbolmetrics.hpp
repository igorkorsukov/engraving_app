#ifndef XTZ_NOTATION_ISYMBOLMETRICS_HPP
#define XTZ_NOTATION_ISYMBOLMETRICS_HPP

#include <map>
#include <unordered_map>

#include "global/io/path.h"
#include "draw/types/geometry.h"
#include "draw/types/font.h"
#include "engraving/types/symid.h"
#include "engraving/style/styledef.h"

namespace xtz::notation {
class ISymbolMetrics
{
public:
    virtual ~ISymbolMetrics() = default;

    virtual void load(const std::string& family, const mu::io::path_t& path) = 0;

    virtual const mu::draw::Font& font() const = 0;

    virtual bool isValid(mu::engraving::SymId id) const = 0;

    virtual char32_t symCode(mu::engraving::SymId id) const = 0;
    virtual mu::engraving::SymId fromCode(char32_t code) const = 0;

    virtual bool isCompound(mu::engraving::SymId id) const = 0;
    virtual const mu::engraving::SymIdList& subSymbols(mu::engraving::SymId id) const = 0; // for compound

    virtual const mu::RectF& bbox(mu::engraving::SymId id) const = 0;
    virtual double advance(mu::engraving::SymId id) const = 0;

    virtual const mu::PointF& smuflAnchor(mu::engraving::SymId symId, mu::engraving::SmuflAnchorId anchorId) const = 0;

    virtual const std::unordered_map<mu::engraving::Sid, mu::engraving::PropertyValue>& engravingDefaults() const = 0;
};
}

#endif // XTZ_NOTATION_ISYMBOLMETRICS_HPP
