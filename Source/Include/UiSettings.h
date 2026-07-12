#pragma once

#include <JuceHeader.h>

namespace UiSettings {
int loadBackgroundColourPreset();
void saveBackgroundColourPreset(int presetId);
int normaliseBackgroundColourPreset(int presetId);

int loadTroubleshootingSetting();
void saveTroubleshootingSetting(int settingId);

int loadHardwareInfoSetting();
void saveHardwareInfoSetting(int settingId);
}  // namespace UiSettings
