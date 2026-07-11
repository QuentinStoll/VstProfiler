#include "UiSettings.h"

#include "SettingsPath.h"

namespace {
const juce::String uiSettingsFileName{"ui_settings.json"};
const juce::Identifier backgroundColourPresetId{"backgroundColourPreset"};

constexpr int defaultBackgroundColourPreset = 1;
constexpr int firstBackgroundColourPreset = 1;
constexpr int lastBackgroundColourPreset = 5;

juce::File getUiSettingsFile() {
    return getFileInSettingsFolder(uiSettingsFileName);
}
}  // namespace

namespace UiSettings {
int normaliseBackgroundColourPreset(int presetId) {
    if (presetId < firstBackgroundColourPreset || presetId > lastBackgroundColourPreset) {
        return defaultBackgroundColourPreset;
    }

    return presetId;
}

int loadBackgroundColourPreset() {
    const auto settingsFile = getUiSettingsFile();
    if (!settingsFile.existsAsFile()) {
        return defaultBackgroundColourPreset;
    }

    auto parsed = juce::JSON::parse(settingsFile.loadFileAsString());
    if (auto* settingsObject = parsed.getDynamicObject()) {
        return normaliseBackgroundColourPreset(static_cast<int>(
            settingsObject->getProperty(backgroundColourPresetId)));
    }

    return defaultBackgroundColourPreset;
}

void saveBackgroundColourPreset(int presetId) {
    auto settingsObject = std::make_unique<juce::DynamicObject>();
    settingsObject->setProperty(backgroundColourPresetId,
                                normaliseBackgroundColourPreset(presetId));

    const auto json = juce::JSON::toString(juce::var(settingsObject.release()), false);

    getUiSettingsFile().replaceWithText(json);
}
}  // namespace UiSettings
