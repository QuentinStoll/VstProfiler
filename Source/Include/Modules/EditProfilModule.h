#pragma once

#include <JuceHeader.h>

class EditProfilModule : public juce::Component {
   public:
    EditProfilModule() = default;
    ~EditProfilModule() override = default;

    void setProfileNumber(int profileNumber);
    int getProfileNumber() const;

    void paint(juce::Graphics& g) override;

   private:
    int _profileNumber = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditProfilModule)
};
