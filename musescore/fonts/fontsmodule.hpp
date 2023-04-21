#ifndef XTZ_FONTS_FONTSMODULE_H
#define XTZ_FONTS_FONTSMODULE_H

#include <global/modularity/imodulesetup.h>

namespace xtz::fonts {
class FontsModule : public mu::modularity::IModuleSetup
{
public:

    std::string moduleName() const override;
    void registerExports() override;
    void onInit(const mu::framework::IApplication::RunMode& mode) override;
};
}

#endif // XTZ_FONTS_FONTSMODULE_H
