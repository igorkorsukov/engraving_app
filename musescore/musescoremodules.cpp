#include "musescoremodules.h"

#include "modularity/ioc.h"
#include "engraving/infrastructure/smufl.h"
#include "engraving/libmscore/mscore.h"
#include "engraving/style/defaultstyle.h"

#include "filesystem.hpp"
#include "resourcesregister.h"
#include "cryptographichash.h"

#include "fonts/fontsmodule.hpp"

#include "engravingconfiguration.hpp"
#include "engravingfontsprovider.hpp"

using namespace mu::modularity;
using namespace mu::engraving;

using namespace xtz::fonts;
using namespace xtz::notation;

std::string moduleName()
{
    return "musescore";
}

void MuseScoreModules::setup()
{
    // Resources
    INIT_RESOURCE(fonts_Edwin);
    INIT_RESOURCE(fonts_MuseScoreTab);
    INIT_RESOURCE(fonts_Bravura);
    INIT_RESOURCE(fonts_Leland);
    INIT_RESOURCE(notations);
    INIT_RESOURCE(smufl);

    // Global
    ioc()->registerExport<mu::io::IFileSystem>(moduleName(), new xtz::io::FileSystem());
    ioc()->registerExport<mu::ICryptographicHash>(moduleName(), new xtz::CryptographicHash());

    // Fonts
    FontsModule fonts;
    fonts.registerExports();
    fonts.onInit(mu::framework::IApplication::RunMode::ConsoleApp);

    // Engraving
    std::shared_ptr<EngravingConfiguration> engravingConf = std::make_shared<EngravingConfiguration>();
    std::shared_ptr<EngravingFontsProvider> engravingFonts = std::make_shared<EngravingFontsProvider>();

    ioc()->registerExport<mu::engraving::IEngravingConfiguration>(moduleName(), engravingConf);
    ioc()->registerExport<mu::engraving::IEngravingFontsProvider>(moduleName(), engravingFonts);

    Smufl::init();
    engravingFonts->addFont("Leland",     "Leland",      ":/fonts/leland/Leland.otf");
    engravingFonts->addFont("Bravura",    "Bravura",     ":/fonts/bravura/Bravura.otf");

    engravingFonts->setFallbackFont("Bravura");

    mu::engraving::MScore::pixelRatio = 1.;

    mu::engraving::MScore::init(); // initialize libmscore

    mu::engraving::DefaultStyle::instance()->init(nullptr,
                                                  nullptr);

    mu::engraving::MScore::setNudgeStep(0.1); // cursor key (default 0.1)
    mu::engraving::MScore::setNudgeStep10(1.0); // Ctrl + cursor key (default 1.0)
    mu::engraving::MScore::setNudgeStep50(0.01); // Alt  + cursor key (default 0.01)
}
