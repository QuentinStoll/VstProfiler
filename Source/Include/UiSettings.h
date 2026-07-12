#pragma once

#include <JuceHeader.h>

namespace UiSettings {
int loadBackgroundColourPreset();
void saveBackgroundColourPreset(int presetId);
int normaliseBackgroundColourPreset(int presetId);
}  // namespace UiSettings
