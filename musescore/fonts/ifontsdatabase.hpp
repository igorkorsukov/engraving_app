#ifndef XTZ_FONTS_IFONTSDATABASE_HPP
#define XTZ_FONTS_IFONTSDATABASE_HPP

#include <string>

// mu
#include "global/modularity/imoduleexport.h"
#include "global/io/path.h"

// xtz
#include "fontstypes.hpp"

namespace xtz::fonts {
class IFontsDatabase : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(xtz::fonts::IFontsDatabase)
public:
    virtual ~IFontsDatabase() = default;

    virtual void setDefaultFont(mu::draw::Font::Type type, const FontDataKey& key) = 0;

    virtual int addFont(const FontDataKey& key, const mu::io::path_t& path) = 0;

    virtual FontDataKey actualFont(const FontDataKey& requireKey, mu::draw::Font::Type type) const = 0;
    virtual FontData fontData(const FontDataKey& requireKey, mu::draw::Font::Type type) const = 0;
    virtual mu::io::path_t fontPath(const FontDataKey& requireKey, mu::draw::Font::Type type) const = 0;

    virtual void addAdditionalFonts(const mu::io::path_t& path) = 0;
};
}

#endif // XTZ_FONTS_IFONTSDATABASE_HPP
