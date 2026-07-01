#pragma once

#include <JuceHeader.h>

#include "Components/ModalOverlay.h"
#include "Components/NotificationBanner.h"
#include "Modules/CardGridModule.h"
#include "Modules/CreateProfilModule.h"
#include "Modules/EditProfilModule.h"
#include "PluginProcessor.h"

class ProfilView : public juce::Component {
   public:
    ProfilView(ProfilerAudioProcessor& p);
    ~ProfilView();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    enum class ContentMode {
        ProfileGrid,
        CreateProfil,
        EditProfil
    };

    juce::Viewport _viewport;
    CardGridModule _grid;
    CreateProfilModule _createProfilModule;
    EditProfilModule _editProfilModule;
    ModalOverlay _modalOverlay;
    NotificationBanner _notificationBanner;
    ProfilerAudioProcessor& _audioProcessor;
    ContentMode _contentMode = ContentMode::ProfileGrid;
    std::unique_ptr<juce::FileChooser> _profileFileChooser;

    void showAddProfileModal();
    void showDeleteProfileModal(int profileNumber);
    void deleteProfile(int profileNumber);
    void showProfileGrid();
    void showCreateProfilModule();
    void showEditProfilModule(int profileNumber);
    void importProfil();
    void refreshProfileGrid();
};
