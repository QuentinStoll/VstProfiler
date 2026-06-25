#include <JuceHeader.h>

#include "Components/CustomImageButton.h"

class CardGridModule : public juce::Component {
   public:
    CardGridModule();
    ~CardGridModule() override;

    std::function<void()> onAddProfileClicked;
    std::function<void(int)> onProfileClicked;

    void setProfileCount(int profileCount);
    int getRequiredHeight(int width) const;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    static constexpr int minColumns = 3;
    static constexpr int maxColumns = 6;
    static constexpr int preferredButtonSize = 200;
    static constexpr int buttonGap = 20;

    int getColumnCount(int buttonCount) const;
    int getColumnCountForWidth(int width, int buttonCount) const;
    int getButtonSize(int columns) const;
    int getButtonSizeForWidth(int width, int columns) const;
    void updateProfileButtons();

    int _profileCount = 10;
    std::vector<std::unique_ptr<CustomImageButton>> _profileButtons;
    std::unique_ptr<CustomImageButton> _addProfileImageButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CardGridModule)
};
