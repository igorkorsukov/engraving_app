#include "engravingconfiguration.hpp"

#include <framework/draw/types/color.h>

using namespace xtz::notation;
using namespace mu::engraving;
using namespace mu;
using namespace mu::draw;

io::path_t EngravingConfiguration::appDataPath() const
{
    return ":/engraving";
}

mu::io::path_t EngravingConfiguration::defaultStyleFilePath() const
{
    return ":";
}

void EngravingConfiguration::setDefaultStyleFilePath(const mu::io::path_t& path)
{
}

mu::io::path_t EngravingConfiguration::partStyleFilePath() const
{
    return {};
}

void EngravingConfiguration::setPartStyleFilePath(const mu::io::path_t& path)
{
}

String EngravingConfiguration::iconsFontFamily() const
{
    return u"";
}

Color EngravingConfiguration::defaultColor() const
{
    return Color::BLACK;
}

Color EngravingConfiguration::scoreInversionColor() const
{
    return Color::BLACK;
}

Color EngravingConfiguration::invisibleColor() const
{
    return "#808080";
}

Color EngravingConfiguration::lassoColor() const
{
    return "#00323200";
}

Color EngravingConfiguration::warningColor() const
{
    return "#808000";
}

Color EngravingConfiguration::warningSelectedColor() const
{
    return "#565600";
}

Color EngravingConfiguration::criticalColor() const
{
    return Color::RED;
}

Color EngravingConfiguration::criticalSelectedColor() const
{
    return "#8B0000";
}

Color EngravingConfiguration::formattingMarksColor() const
{
    return "#A0A0A4";
}

mu::draw::Color EngravingConfiguration::thumbnailBackgroundColor() const
{
    return mu::draw::Color::WHITE;
}

mu::draw::Color EngravingConfiguration::noteBackgroundColor() const
{
    return mu::draw::Color::WHITE;
}

double EngravingConfiguration::guiScaling() const
{
    return 1.0;
}

Color EngravingConfiguration::selectionColor(mu::engraving::voice_idx_t voiceIndex, bool itemVisible) const
{
    return Color::WHITE;
}

void EngravingConfiguration::setSelectionColor(voice_idx_t voiceIndex, Color color)
{
}

bool EngravingConfiguration::scoreInversionEnabled() const
{
    return false;
}

void EngravingConfiguration::setScoreInversionEnabled(bool value)
{
}

mu::async::Notification EngravingConfiguration::scoreInversionChanged() const
{
    return mu::async::Notification();
}

mu::async::Channel<voice_idx_t, Color> EngravingConfiguration::selectionColorChanged() const
{
    return mu::async::Channel<voice_idx_t, Color>();
}

Color EngravingConfiguration::highlightSelectionColor(voice_idx_t voice) const
{
    return Color::WHITE;
}

const mu::engraving::IEngravingConfiguration::DebuggingOptions& EngravingConfiguration::debuggingOptions() const
{
    static mu::engraving::IEngravingConfiguration::DebuggingOptions opt;
    return opt;
}

void EngravingConfiguration::setDebuggingOptions(const DebuggingOptions& options)
{
}

mu::async::Notification EngravingConfiguration::debuggingOptionsChanged() const
{
    return mu::async::Notification();
}

bool EngravingConfiguration::isAccessibleEnabled() const
{
    return false;
}

bool EngravingConfiguration::guitarProImportExperimental() const
{
    return true;
}

bool EngravingConfiguration::negativeFretsAllowed() const
{
    return true;
}

bool EngravingConfiguration::tablatureParenthesesZIndexWorkaround() const
{
    return true;
}

bool EngravingConfiguration::crossNoteHeadAlwaysBlack() const
{
    return true;
}

bool EngravingConfiguration::enableExperimentalFretCircle() const
{
    return false;
}

void EngravingConfiguration::setGuitarProMultivoiceEnabled(bool multiVoice)
{
    m_multiVoice = multiVoice;
}

bool EngravingConfiguration::guitarProMultivoiceEnabled() const
{
    return m_multiVoice;
}

bool EngravingConfiguration::minDistanceForPartialSkylineCalculated() const
{
    return true;
}
