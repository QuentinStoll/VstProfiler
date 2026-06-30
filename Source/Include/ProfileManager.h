#pragma once

#include <JuceHeader.h>

#include <optional>
#include <vector>

struct Profile {
    // Stable id is also the on-disk filename, so display names can change freely.
    juce::String id;
    juce::String name;
    juce::File file;
    juce::String createdAt;
    juce::String updatedAt;
    juce::NamedValueSet values;
};

class ProfileManager : public juce::ChangeBroadcaster {
   public:
    static constexpr const char* profileFileExtension = ".profilerprofile";

    // Creates the manager and loads profiles/settings from disk.
    explicit ProfileManager(juce::AudioProcessorValueTreeState& apvts,
                            juce::File profileDirectory = {});

    // Rebuilds the in-memory profile list from `.profilerprofile` files.
    void refreshProfiles();

    // Returns how many valid profiles are currently loaded.
    int getProfileCount() const noexcept;

    // Returns profile display names in the same order as the loaded profiles.
    juce::StringArray getProfileNames() const;

    // Gives read-only access to all loaded profile records.
    const std::vector<Profile>& getProfiles() const noexcept;

    // Returns a profile by UI/list index, or nullptr if the index is invalid.
    const Profile* getProfile(int profileIndex) const;

    // Returns saved form/APVTS values for a profile, or an empty set if invalid.
    juce::NamedValueSet getProfileValues(int profileIndex) const;

    // Returns the folder where profile files are stored.
    juce::File getProfileDirectory() const;

    // Returns the profile id saved as the Play view's current profile.
    juce::String getCurrentProfileId() const;

    // Resolves the saved current profile id back to the loaded profile index.
    int getCurrentProfileIndex() const;

    // Mutating operations refresh the in-memory list and broadcast changes.
    // Creates a new UUID-named profile file from form values.
    bool createProfile(const juce::NamedValueSet& values,
                       juce::String* errorMessage = nullptr);

    // Updates an existing profile file without changing its UUID.
    bool updateProfile(int profileIndex,
                       const juce::NamedValueSet& values,
                       juce::String* errorMessage = nullptr);

    // Deletes a profile file and clears it if it was the current profile.
    bool deleteProfile(int profileIndex,
                       juce::String* errorMessage = nullptr);

    // Imports an external profile file and assigns it a fresh local UUID.
    bool importProfile(const juce::File& sourceFile,
                       juce::String* errorMessage = nullptr);

    // Applies only APVTS-backed values; processor-level apply handles IR files.
    // Applies a loaded profile's stored parameter values.
    bool applyProfile(int profileIndex,
                      juce::String* errorMessage = nullptr);

    // Applies the profile remembered as the current Play view profile.
    bool applyCurrentProfile(juce::String* errorMessage = nullptr);

    // Applies a profile and persists it as the Play view's current profile.
    bool applyProfileAndSaveAsCurrent(int profileIndex,
                                      juce::String* errorMessage = nullptr);

    // Persists a loaded profile id as the current Play view profile.
    void setCurrentProfileId(const juce::String& profileId);

    // Clears the persisted current Play view profile id.
    void clearCurrentProfile();

   private:
    juce::AudioProcessorValueTreeState& _apvts;
    juce::File _profileDirectory;

    // Persisted in play_view_settings.json so the Play view can restore itself.
    juce::String _currentProfileId;
    std::vector<Profile> _profiles;

    // Ensures the profile directory exists before file operations.
    bool ensureProfileDirectory(juce::String* errorMessage) const;

    // Serialises one Profile object to its JSON `.profilerprofile` file.
    bool writeProfileFile(const Profile& profile,
                          juce::String* errorMessage) const;

    // Parses a `.profilerprofile` file into a Profile object.
    std::optional<Profile> readProfileFile(const juce::File& file,
                                           juce::String* errorMessage) const;

    // Builds the full file path for a profile UUID.
    juce::File getFileForId(const juce::String& profileId) const;

    // Generates the default display name for a newly created profile.
    juce::String createDefaultProfileName() const;

    // New profile files are UUID-named to avoid coupling filenames to UI labels.
    juce::String createUniqueProfileId() const;

    // Returns the current timestamp for profile metadata.
    static juce::String getTimestamp();

    // Extracts and sanitises the display name from form values.
    static juce::String getProfileNameFromValues(const juce::NamedValueSet& values,
                                                 const juce::String& fallbackName);

    // Ensures the profile values always carry the display name.
    static juce::NamedValueSet normaliseProfileValues(const juce::NamedValueSet& values,
                                                      const juce::String& profileName);

    // Rejects invalid known profile fields before they can be saved or applied.
    static bool validateProfileValues(const juce::NamedValueSet& values,
                                      juce::String* errorMessage);

    // Writes an error message only when the caller provided storage for it.
    static void setError(juce::String* errorMessage, const juce::String& message);

    // Loads the Play view's last selected profile id from settings.
    void loadPlayViewSettings();

    // Saves the Play view's last selected profile id to settings.
    void savePlayViewSettings() const;

    // Converts a real parameter value to normalised APVTS space and applies it.
    void setParameterValue(const juce::String& parameterId, float value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProfileManager)
};
