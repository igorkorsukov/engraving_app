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

#ifndef XTZ_NOTATION_ENGRAVINGFONTSPROVIDER_H
#define XTZ_NOTATION_ENGRAVINGFONTSPROVIDER_H

#include <vector>

#include <engraving/iengravingfontsprovider.h>

namespace xtz::notation {
class EngravingFont;
class EngravingFontsProvider : public mu::engraving::IEngravingFontsProvider
{
public:

    void addFont(const std::string& name, const std::string& family, const mu::io::path_t& filePath) override;
    mu::engraving::IEngravingFontPtr fontByName(const std::string& name) const override;
    std::vector<mu::engraving::IEngravingFontPtr> fonts() const override;

    void setFallbackFont(const std::string& name) override;
    mu::engraving::IEngravingFontPtr fallbackFont() const override;
    bool isFallbackFont(const mu::engraving::IEngravingFont* f) const override;
    void loadAllFonts() override;

    // Dev
    void clear();

private:

    std::shared_ptr<EngravingFont> doFontByName(const std::string& name) const;
    std::shared_ptr<EngravingFont> doFallbackFont() const;

    struct Fallback {
        std::string name;
        std::shared_ptr<EngravingFont> font;
    };

    mutable Fallback m_fallback;
    std::vector<std::shared_ptr<EngravingFont> > m_engravingFonts;
};
}

#endif // XTZ_NOTATION_ENGRAVINGFONTSPROVIDER_H
