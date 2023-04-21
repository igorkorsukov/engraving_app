#ifndef XTZ_NOTATION_ENGRAVINGCONFIGURATION_HPP
#define XTZ_NOTATION_ENGRAVINGCONFIGURATION_HPP

#include <engraving/iengravingconfiguration.h>
#include <global/async/channel.h>
#include <global/async/notification.h>

namespace xtz::notation {
class EngravingConfiguration : public mu::engraving::IEngravingConfiguration
{
public:
    EngravingConfiguration() = default;

    mu::io::path_t appDataPath() const override;

    mu::io::path_t defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const mu::io::path_t& path) override;

    mu::io::path_t partStyleFilePath() const override;
    void setPartStyleFilePath(const mu::io::path_t& path) override;

    mu::String iconsFontFamily() const override;
    mu::SizeF defaultPageSize() const override { return mu::SizeF(); }

    mu::draw::Color defaultColor() const override;
    mu::draw::Color scoreInversionColor() const override;
    mu::draw::Color invisibleColor() const override;
    mu::draw::Color lassoColor() const override;
    mu::draw::Color warningColor() const override;
    mu::draw::Color warningSelectedColor() const override;
    mu::draw::Color criticalColor() const override;
    mu::draw::Color criticalSelectedColor() const override;
    mu::draw::Color formattingMarksColor() const override;
    mu::draw::Color thumbnailBackgroundColor() const override;
    mu::draw::Color noteBackgroundColor() const override;

    double guiScaling() const override;

    mu::draw::Color selectionColor(mu::engraving::voice_idx_t voiceIndex = 0, bool itemVisible = true) const override;
    void setSelectionColor(mu::engraving::voice_idx_t voiceIndex, mu::draw::Color color) override;
    mu::async::Channel<mu::engraving::voice_idx_t, mu::draw::Color> selectionColorChanged() const override;

    bool scoreInversionEnabled() const override;
    void setScoreInversionEnabled(bool value) override;
    mu::async::Notification scoreInversionChanged() const override;

    mu::draw::Color highlightSelectionColor(mu::engraving::voice_idx_t voice = 0) const override;

    const DebuggingOptions& debuggingOptions() const override;
    void setDebuggingOptions(const DebuggingOptions& options) override;
    mu::async::Notification debuggingOptionsChanged() const override;

    bool isAccessibleEnabled() const override;

    bool guitarProImportExperimental() const override;
    bool negativeFretsAllowed() const override;
    bool tablatureParenthesesZIndexWorkaround() const override;
    bool crossNoteHeadAlwaysBlack() const override;
    bool enableExperimentalFretCircle() const override;

    void setGuitarProMultivoiceEnabled(bool multiVoice) override;
    bool guitarProMultivoiceEnabled() const override;
    bool minDistanceForPartialSkylineCalculated() const override;

private:
    bool m_multiVoice = false;
};
}

#endif//XTZ_NOTATION_ENGRAVINGCONFIGURATION_HPP
