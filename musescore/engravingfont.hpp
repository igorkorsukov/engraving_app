/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef XTZ_NOTATION_ENGRAVINGFONT_H
#define XTZ_NOTATION_ENGRAVINGFONT_H

#include <unordered_map>
#include <functional>

#include <engraving/iengravingfont.h>

#include "modularity/ioc.h"
#include "draw/types/geometry.h"
#include <engraving/iengravingfontsprovider.h>

#include "global/io/path.h"

#include "style/styledef.h"
#include "types/symid.h"

namespace mu {
class JsonObject;
}

namespace mu::draw {
class Painter;
}

namespace xtz::notation {
class ISymbolMetrics;
class EngravingFont : public mu::engraving::IEngravingFont
{
    INJECT_STATIC(score, mu::engraving::IEngravingFontsProvider, engravingFonts)
public:
    EngravingFont(const std::string& name, const std::string& family, const mu::io::path_t& filePath);
    ~EngravingFont();

    const std::string& name() const override;
    const std::string& family() const override;

    std::unordered_map<mu::engraving::Sid, mu::engraving::PropertyValue> engravingDefaults() const override;

    char32_t symCode(mu::engraving::SymId id) const override;
    mu::engraving::SymId fromCode(char32_t code) const override;
    mu::String toString(mu::engraving::SymId id) const override;

    bool isValid(mu::engraving::SymId id) const override;

    mu::RectF bbox(mu::engraving::SymId id, double mag) const override;
    mu::RectF bbox(mu::engraving::SymId id, const mu::SizeF&) const override;
    mu::RectF bbox(const mu::engraving::SymIdList& s, double mag) const override;
    mu::RectF bbox(const mu::engraving::SymIdList& s, const mu::SizeF& mag) const override;

    double width(mu::engraving::SymId id, double mag) const override;
    double width(const mu::engraving::SymIdList&, double mag) const override;
    double height(mu::engraving::SymId id, double mag) const override;
    double advance(mu::engraving::SymId id, double mag) const override;

    mu::PointF smuflAnchor(mu::engraving::SymId symId, mu::engraving::SmuflAnchorId anchorId, double mag) const override;

    // Draw
    void draw(mu::engraving::SymId id, mu::draw::Painter* p, double mag, const mu::PointF& pos) const override;
    void draw(mu::engraving::SymId id, mu::draw::Painter* p, const mu::SizeF& mag, const mu::PointF& pos) const override;

    void draw(const mu::engraving::SymIdList& ids, mu::draw::Painter* p, double mag, const mu::PointF& pos) const override;
    void draw(const mu::engraving::SymIdList& ids, mu::draw::Painter* p, const mu::SizeF& mag, const mu::PointF& pos) const override;

    void ensureLoad();

    // Dev
    using MetricsFatory = std::function<ISymbolMetrics* (const mu::io::path_t&)>;
    static void setMetricsFactory(const MetricsFatory& f);

private:

    friend class SymbolFonts;

    bool useFallbackFont(mu::engraving::SymId id) const;

    bool m_loaded = false;

    std::string m_name;
    std::string m_family;
    mu::io::path_t m_fontPath;
    static MetricsFatory s_metricsFactory;
    ISymbolMetrics* m_face = nullptr;
};
}

#endif // XTZ_NOTATION_ENGRAVINGFONT_H
