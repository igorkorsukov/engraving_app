#include "fontsdatabase.hpp"

// mu
#include "global/io/file.h"
#include "global/io/dir.h"
#include "global/serialization/json.h"

// xtz
#include "log.h"

using namespace xtz::fonts;

static int s_fontID = -1;

void FontsDatabase::setDefaultFont(mu::draw::Font::Type type, const FontDataKey& key)
{
    m_defaults[type] = key;
}

const FontDataKey& FontsDatabase::defaultFont(mu::draw::Font::Type type) const
{
    auto it = m_defaults.find(type);
    if (it != m_defaults.end()) {
        return it->second;
    }

    it = m_defaults.find(mu::draw::Font::Type::Unknown);
    IF_ASSERT_FAILED(it != m_defaults.end()) {
        static FontDataKey null;
        return null;
    }
    return it->second;
}

int FontsDatabase::addFont(const FontDataKey& key, const mu::io::path_t& path)
{
    s_fontID++;
    m_fonts.push_back(FontInfo { s_fontID, key, path });
    return s_fontID;
}

FontDataKey FontsDatabase::actualFont(const FontDataKey& requireKey, mu::draw::Font::Type type) const
{
    mu::io::path_t path = fontInfo(requireKey).path;
    if (!path.empty() && mu::io::File::exists(path)) {
        return requireKey;
    }

    return defaultFont(type);
}

FontData FontsDatabase::fontData(const FontDataKey& requireKey, mu::draw::Font::Type type) const
{
    FontDataKey key = actualFont(requireKey, type);
    mu::io::path_t path = fontInfo(key).path;
    IF_ASSERT_FAILED(mu::io::File::exists(path)) {
        return FontData();
    }

    mu::io::File file(path);
    if (!file.open()) {
        LOGE() << "failed open font file: " << path;
        return FontData();
    }

    FontData fd;
    fd.key = key;
    fd.data = file.readAll();
    return fd;
}

mu::io::path_t FontsDatabase::fontPath(const FontDataKey& requireKey, mu::draw::Font::Type type) const
{
    FontDataKey key = actualFont(requireKey, type);
    mu::io::path_t path = fontInfo(key).path;
    IF_ASSERT_FAILED(mu::io::File::exists(path)) {
        return mu::io::path_t();
    }
    return path;
}

const FontsDatabase::FontInfo& FontsDatabase::fontInfo(const FontDataKey& key) const
{
    for (const FontInfo& fi : m_fonts) {
        if (fi.key == key) {
            return fi;
        }
    }

    static FontInfo null;
    return null;
}

void FontsDatabase::addAdditionalFonts(const mu::io::path_t& path)
{
    mu::io::File f(path + "/fontslist.json");
    if (!f.open(mu::io::IODevice::ReadOnly)) {
        LOGE() << "failed open file: " << f.filePath();
        return;
    }

    mu::io::path_t absolutePath = mu::io::Dir(path).absolutePath() + "/";

    mu::ByteArray data = f.readAll();
    std::string err;
    mu::JsonDocument json = mu::JsonDocument::fromJson(data, &err);
    if (!err.empty()) {
        LOGE() << "failed parse: " << f.filePath();
        return;
    }

    mu::JsonArray fontInfos = json.rootArray();
    for (size_t i = 0; i < fontInfos.size(); ++i) {
        mu::JsonObject infoObj = fontInfos.at(i).toObject();

        std::string file = infoObj.value("file").toStdString();
        if (file.empty()) {
            continue;
        }
        std::string family = infoObj.value("family").toStdString();
        if (family.empty()) {
            continue;
        }
        bool bold = infoObj.value("bold").toBool();
        bool italic = infoObj.value("italic").toBool();

        addFont(FontDataKey(family, bold, italic), absolutePath + file);
    }
}
