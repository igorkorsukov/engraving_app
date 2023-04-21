#include "musescore/musescoremodules.h"

#include "engraving/libmscore/score.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/mscxcompat.h"

#include "log.h"

int main()
{
    MuseScoreModules::setup();

    std::string scorePath = std::string(SOURCE_PATH) + "/simple.mscz";

    mu::engraving::MasterScore* score = mu::engraving::compat::ScoreAccess::createMasterScore();
    mu::Ret ret = mu::engraving::compat::loadMsczOrMscx(score, mu::String::fromStdString(scorePath), false);
    if (ret) {
        LOGI() << "success score loaded";
    } else {
        LOGE() << "failed score loaded, err: " << ret.toString();
    }
}
