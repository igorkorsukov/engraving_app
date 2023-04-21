#include "fontsmodule.hpp"

#include "global/modularity/ioc.h"

#include "draw/ifontprovider.h" // mu

#include "internal/fontprovider.hpp"
#include "internal/fontsdatabase.hpp"
#include "internal/fontsengine.hpp"

using namespace mu::modularity;
using namespace xtz::fonts;

static std::shared_ptr<FontsEngine> s_fontsEngine = nullptr;

std::string FontsModule::moduleName() const
{
    return "xtz_fonts";
}

void FontsModule::registerExports()
{
    s_fontsEngine = std::make_shared<FontsEngine>();

    ioc()->registerExport<IFontsDatabase>(moduleName(), new FontsDatabase());
    ioc()->registerExport<IFontsEngine>(moduleName(), s_fontsEngine);
    ioc()->registerExport<mu::draw::IFontProvider>(moduleName(), new FontProvider());
}

void FontsModule::onInit(const mu::framework::IApplication::RunMode&)
{
    using namespace mu::draw;

    std::shared_ptr<IFontsDatabase> fdb = ioc()->resolve<IFontsDatabase>(moduleName());

    // Text
    fdb->addFont(FontDataKey("Edwin", false, false), ":/fonts/edwin/Edwin-Roman.otf");
    fdb->addFont(FontDataKey("Edwin", false, true), ":/fonts/edwin/Edwin-Italic.otf");
    fdb->addFont(FontDataKey("Edwin", true, false), ":/fonts/edwin/Edwin-Bold.otf");
    fdb->addFont(FontDataKey("Edwin", true, true), ":/fonts/edwin/Edwin-BdIta.otf");

    // MusicSymbol[Text]
#ifdef XTZ_USE_FTX_FONTS
    fdb->addFont(FontDataKey("Bravura"), ":/fonts/bravura/Bravura.ftx");
    fdb->addFont(FontDataKey("Bravura Text"), ":/fonts/bravura/BravuraText.ftx");
    fdb->addFont(FontDataKey("Leland"), ":/fonts/leland/Leland.ftx");
    fdb->addFont(FontDataKey("Leland Text"), ":/fonts/leland/LelandText.ftx");
#else
    fdb->addFont(FontDataKey("Bravura"), ":/fonts/bravura/Bravura.otf");
    fdb->addFont(FontDataKey("Bravura Text"), ":/fonts/bravura/BravuraText.otf");
    fdb->addFont(FontDataKey("Leland"), ":/fonts/leland/Leland.otf");
    fdb->addFont(FontDataKey("Leland Text"), ":/fonts/leland/LelandText.otf");
#endif

    // Tabulature
#ifdef XTZ_USE_FTX_FONTS
    fdb->addFont(FontDataKey("MuseScoreTab"), ":/fonts/tab/MuseScoreTab.ftx");
#else
    fdb->addFont(FontDataKey("MuseScoreTab"), ":/fonts/MuseScoreTab.ttf");
#endif

    fdb->setDefaultFont(Font::Type::Unknown, FontDataKey("Edwin"));
    fdb->setDefaultFont(Font::Type::Text, FontDataKey("Edwin"));
    fdb->setDefaultFont(Font::Type::MusicSymbolText, FontDataKey("Bravura Text"));
    fdb->setDefaultFont(Font::Type::MusicSymbol, FontDataKey("Bravura"));
    fdb->setDefaultFont(Font::Type::Tablature, FontDataKey("MuseScoreTab"));

    s_fontsEngine->init();
}
