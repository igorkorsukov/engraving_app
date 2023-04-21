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
#include "engravingfont.hpp"

#include "draw/painter.h"

#include "libmscore/mscore.h"

#include "symbolmetricsfm.hpp"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::draw;
using namespace mu::engraving;
using namespace xtz::notation;

EngravingFont::MetricsFatory EngravingFont::s_metricsFactory = nullptr;

void EngravingFont::setMetricsFactory(const MetricsFatory& f)
{
    s_metricsFactory = f;
}

EngravingFont::EngravingFont(const std::string& name, const std::string& family, const path_t& filePath)
    : m_name(name), m_family(family), m_fontPath(filePath)
{
    if (s_metricsFactory) {
        m_face = s_metricsFactory(m_fontPath);
    } else {
#ifdef XTZ_USE_SMX_FONTS
        m_face = new SymbolMetricsXT();
#else
        m_face = new SymbolMetricsFM();
#endif
    }
}

EngravingFont::~EngravingFont()
{
    delete m_face;
}

const std::string& EngravingFont::name() const
{
    return m_name;
}

const std::string& EngravingFont::family() const
{
    return m_family;
}

std::unordered_map<Sid, PropertyValue> EngravingFont::engravingDefaults() const
{
    return m_face->engravingDefaults();
}

void EngravingFont::ensureLoad()
{
    if (m_loaded) {
        return;
    }

    m_face->load(m_family, m_fontPath);

    m_loaded = true;
}

char32_t EngravingFont::symCode(SymId id) const
{
    return m_face->symCode(id);
}

SymId EngravingFont::fromCode(char32_t code) const
{
    return m_face->fromCode(code);
}

static String codeToString(char32_t code)
{
    return String::fromUcs4(&code, 1);
}

String EngravingFont::toString(SymId id) const
{
    return codeToString(symCode(id));
}

bool EngravingFont::isValid(SymId id) const
{
    return m_face->isValid(id);
}

bool EngravingFont::useFallbackFont(SymId id) const
{
    return MScore::useFallbackFont && !isValid(id) && !engravingFonts()->isFallbackFont(this);
}

// =============================================
// Symbol bounding box
// =============================================

RectF EngravingFont::bbox(SymId id, double mag) const
{
    return bbox(id, SizeF(mag, mag));
}

RectF EngravingFont::bbox(SymId id, const SizeF& mag) const
{
    if (useFallbackFont(id)) {
        return engravingFonts()->fallbackFont()->bbox(id, mag);
    }

    RectF r = m_face->bbox(id);
    return RectF(r.x() * mag.width(), r.y() * mag.height(),
                 r.width() * mag.width(), r.height() * mag.height());
}

RectF EngravingFont::bbox(const SymIdList& s, double mag) const
{
    return bbox(s, SizeF(mag, mag));
}

RectF EngravingFont::bbox(const SymIdList& s, const SizeF& mag) const
{
    RectF r;
    PointF pos;
    for (SymId id : s) {
        r.unite(bbox(id, mag).translated(pos));
        pos.rx() += advance(id, mag.width());
    }
    return r;
}

// =============================================
// Symbol metrics
// =============================================

double EngravingFont::width(SymId id, double mag) const
{
    return bbox(id, mag).width();
}

double EngravingFont::height(SymId id, double mag) const
{
    return bbox(id, mag).height();
}

double EngravingFont::advance(SymId id, double mag) const
{
    if (useFallbackFont(id)) {
        return engravingFonts()->fallbackFont()->advance(id, mag);
    }

    return m_face->advance(id) * mag;
}

double EngravingFont::width(const SymIdList& s, double mag) const
{
    return bbox(s, mag).width();
}

PointF EngravingFont::smuflAnchor(SymId symId, SmuflAnchorId anchorId, double mag) const
{
    if (useFallbackFont(symId)) {
        return engravingFonts()->fallbackFont()->smuflAnchor(symId, anchorId, mag);
    }

    return m_face->smuflAnchor(symId, anchorId) * mag;
}

// =============================================
// Draw
// =============================================

void EngravingFont::draw(SymId id, Painter* painter, const SizeF& mag, const PointF& pos) const
{
    if (m_face->isCompound(id)) { // is this a compound symbol?
        draw(m_face->subSymbols(id), painter, mag, pos);
        return;
    }

    if (!m_face->isValid(id)) {
        if (MScore::useFallbackFont && !engravingFonts()->isFallbackFont(this)) {
            engravingFonts()->fallbackFont()->draw(id, painter, mag, pos);
        } else {
            LOGE() << "invalid sym: " << static_cast<size_t>(id);
        }

        return;
    }

    painter->save();
    painter->scale(mag.width(), mag.height());
    painter->setFont(m_face->font());
    painter->drawSymbol(PointF(pos.x() / mag.width(), pos.y() / mag.height()), symCode(id));
    painter->restore();
}

void EngravingFont::draw(SymId id, Painter* painter, double mag, const PointF& pos) const
{
    draw(id, painter, SizeF(mag, mag), pos);
}

void EngravingFont::draw(const SymIdList& ids, Painter* painter, double mag, const PointF& startPos) const
{
    PointF pos(startPos);
    for (SymId id : ids) {
        draw(id, painter, mag, pos);
        pos.setX(pos.x() + advance(id, mag));
    }
}

void EngravingFont::draw(const SymIdList& ids, Painter* painter, const SizeF& mag, const PointF& startPos) const
{
    PointF pos(startPos);
    for (SymId id : ids) {
        draw(id, painter, mag, pos);
        pos.setX(pos.x() + advance(id, mag.width()));
    }
}
