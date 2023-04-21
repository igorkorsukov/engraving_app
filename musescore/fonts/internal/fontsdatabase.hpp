#ifndef XTZ_FONTS_FONTSDATABASE_H
#define XTZ_FONTS_FONTSDATABASE_H

#include <vector>
#include <map>

// xtz
#include "../ifontsdatabase.hpp"

namespace xtz::fonts {
class FontsDatabase : public IFontsDatabase
{
public:
    FontsDatabase() = default;

    void setDefaultFont(mu::draw::Font::Type type, const FontDataKey& key) override;

    int addFont(const FontDataKey& key, const mu::io::path_t& path) override;

    FontDataKey actualFont(const FontDataKey& requireKey, mu::draw::Font::Type type) const override;
    FontData fontData(const FontDataKey& requireKey, mu::draw::Font::Type type) const override;
    mu::io::path_t fontPath(const FontDataKey& requireKey, mu::draw::Font::Type type) const override;

    void addAdditionalFonts(const mu::io::path_t& path) override;

private:

    struct FontInfo {
        int id = -1;
        FontDataKey key;
        mu::io::path_t path;

        bool valid() const { return id > -1; }
    };

    const FontDataKey& defaultFont(mu::draw::Font::Type type) const;
    const FontInfo& fontInfo(const FontDataKey& key) const;

    std::map<mu::draw::Font::Type, FontDataKey> m_defaults;
    std::vector<FontInfo> m_fonts;
};
}
#endif // XTZ_FONTS_FONTSDATABASE_H
