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

#include "engravingfontsprovider.hpp"

#include "global/stringutils.h"

#include "engravingfont.hpp"

#include "log.h"

using namespace xtz::notation;

void EngravingFontsProvider::addFont(const std::string& name, const std::string& family, const mu::io::path_t& filePath)
{
    m_engravingFonts.push_back(std::make_shared<EngravingFont>(name, family, filePath));
    m_fallback.font = nullptr;
}

std::shared_ptr<EngravingFont> EngravingFontsProvider::doFontByName(const std::string& name) const
{
    std::string name_lo = mu::strings::toLower(name);
    for (const std::shared_ptr<EngravingFont>& f : m_engravingFonts) {
        if (mu::strings::toLower(f->name()) == name_lo) {
            return f;
        }
    }
    return nullptr;
}

mu::engraving::IEngravingFontPtr EngravingFontsProvider::fontByName(const std::string& name) const
{
    std::shared_ptr<EngravingFont> font = doFontByName(name);
    if (!font) {
        font = doFallbackFont();
    }

    font->ensureLoad();
    return font;
}

std::vector<mu::engraving::IEngravingFontPtr> EngravingFontsProvider::fonts() const
{
    std::vector<mu::engraving::IEngravingFontPtr> fs;
    for (const std::shared_ptr<EngravingFont>& f : m_engravingFonts) {
        fs.push_back(f);
    }
    return fs;
}

void EngravingFontsProvider::setFallbackFont(const std::string& name)
{
    m_fallback.name = name;
    m_fallback.font = nullptr;
}

std::shared_ptr<EngravingFont> EngravingFontsProvider::doFallbackFont() const
{
    if (!m_fallback.font) {
        m_fallback.font = doFontByName(m_fallback.name);
        IF_ASSERT_FAILED(m_fallback.font) {
            return nullptr;
        }
    }

    return m_fallback.font;
}

mu::engraving::IEngravingFontPtr EngravingFontsProvider::fallbackFont() const
{
    std::shared_ptr<EngravingFont> font = doFallbackFont();
    font->ensureLoad();
    return font;
}

bool EngravingFontsProvider::isFallbackFont(const mu::engraving::IEngravingFont* f) const
{
    return doFallbackFont().get() == f;
}

void EngravingFontsProvider::loadAllFonts()
{
}

void EngravingFontsProvider::clear()
{
    m_fallback.font = nullptr;
    m_engravingFonts.clear();
}
