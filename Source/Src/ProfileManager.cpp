#include "ProfileManager.h"

#include <algorithm>

#include "SettingsPath.h"

namespace {
// JSON keys shared by profile files and the small Play view settings file.
const juce::Identifier formatId{"format"};
const juce::Identifier versionId{"version"};
const juce::Identifier idId{"id"};
const juce::Identifier nameId{"name"};
const juce::Identifier createdAtId{"createdAt"};
const juce::Identifier updatedAtId{"updatedAt"};
const juce::Identifier parametersId{"parameters"};
const juce::Identifier profileNameId{"profileName"};
const juce::Identifier currentProfileId{"currentProfileId"};

const juce::String profileFormat{"profilerprofile"};
constexpr int profileFormatVersion = 1;
const juce::String playViewSettingsFileName{"play_view_settings.json"};

// Returns the settings-backed folder that owns all profile files.
juce::File getDefaultProfileDirectory() {
    // Profiles live with other plugin settings, not in the JUCE cache.
    return getSettingsFolder()
        .getChildFile("Profiles");
}

// Returns the settings file that stores the Play view's last selected profile.
juce::File getPlayViewSettingsFile() {
    return getFileInSettingsFolder(playViewSettingsFileName);
}
}  // namespace

// Initialises storage locations and loads both profile files and Play view state.
ProfileManager::ProfileManager(juce::AudioProcessorValueTreeState& apvts,
                               juce::File profileDirectory)
    : _apvts(apvts),
      _profileDirectory(profileDirectory == juce::File{} ? getDefaultProfileDirectory()
                                                         : std::move(profileDirectory)) {
    refreshProfiles();
    loadPlayViewSettings();
}

// Reloads all valid profile files from disk into the in-memory list.
void ProfileManager::refreshProfiles() {
    _profiles.clear();

    if (!ensureProfileDirectory(nullptr)) {
        return;
    }

    auto files = _profileDirectory.findChildFiles(juce::File::findFiles,
                                                  false,
                                                  "*" + juce::String(profileFileExtension));

    // Keep the UI order stable between refreshes without storing a separate index.
    std::sort(files.begin(), files.end(), [](const juce::File& left, const juce::File& right) {
        return left.getFileName().compareIgnoreCase(right.getFileName()) < 0;
    });

    for (const auto& file : files) {
        if (auto profile = readProfileFile(file, nullptr)) {
            _profiles.push_back(*profile);
        }
    }
}

// Returns the number of profiles currently available to the UI.
int ProfileManager::getProfileCount() const noexcept {
    return static_cast<int>(_profiles.size());
}

// Returns only the profile display names, preserving profile-list order.
juce::StringArray ProfileManager::getProfileNames() const {
    juce::StringArray names;

    for (const auto& profile : _profiles) {
        names.add(profile.name);
    }

    return names;
}

// Exposes all loaded profile records for read-only UI/model queries.
const std::vector<Profile>& ProfileManager::getProfiles() const noexcept {
    return _profiles;
}

// Resolves a UI/list index to a profile record.
const Profile* ProfileManager::getProfile(int profileIndex) const {
    if (profileIndex < 0 || profileIndex >= getProfileCount()) {
        return nullptr;
    }

    return &_profiles[static_cast<size_t>(profileIndex)];
}

// Returns the saved parameter/form values for a profile.
juce::NamedValueSet ProfileManager::getProfileValues(int profileIndex) const {
    if (const auto* profile = getProfile(profileIndex)) {
        return profile->values;
    }

    return {};
}

// Returns the directory used for creating/importing profile files.
juce::File ProfileManager::getProfileDirectory() const {
    return _profileDirectory;
}

// Returns the persisted current profile id used by the Play view.
juce::String ProfileManager::getCurrentProfileId() const {
    return _currentProfileId;
}

// Converts the persisted current profile id back into the current list index.
int ProfileManager::getCurrentProfileIndex() const {
    if (_currentProfileId.isEmpty()) {
        return -1;
    }

    // Resolve the persisted id lazily so deletes/imports cannot leave a stale index.
    for (int index = 0; index < static_cast<int>(_profiles.size()); ++index) {
        if (_profiles[static_cast<size_t>(index)].id == _currentProfileId) {
            return index;
        }
    }

    return -1;
}

// Creates a new profile JSON file with a UUID filename.
bool ProfileManager::createProfile(const juce::NamedValueSet& values,
                                   juce::String* errorMessage) {
    if (!ensureProfileDirectory(errorMessage)) {
        return false;
    }

    const auto profileName = getProfileNameFromValues(values, createDefaultProfileName());
    const auto profileId = createUniqueProfileId();
    const auto timestamp = getTimestamp();

    // The profile name is user-facing; the UUID id is the stable file identity.
    Profile profile;
    profile.id = profileId;
    profile.name = profileName;
    profile.file = getFileForId(profileId);
    profile.createdAt = timestamp;
    profile.updatedAt = timestamp;
    profile.values = normaliseProfileValues(values, profileName);

    if (!writeProfileFile(profile, errorMessage)) {
        return false;
    }

    refreshProfiles();
    sendChangeMessage();
    return true;
}

// Updates a profile's saved values while keeping its UUID and file path stable.
bool ProfileManager::updateProfile(int profileIndex,
                                   const juce::NamedValueSet& values,
                                   juce::String* errorMessage) {
    const auto* existingProfile = getProfile(profileIndex);
    if (existingProfile == nullptr) {
        setError(errorMessage, "Profile not found.");
        return false;
    }

    auto updatedProfile = *existingProfile;
    updatedProfile.name = getProfileNameFromValues(values, updatedProfile.name);
    updatedProfile.updatedAt = getTimestamp();
    updatedProfile.values = normaliseProfileValues(values, updatedProfile.name);

    if (!writeProfileFile(updatedProfile, errorMessage)) {
        return false;
    }

    refreshProfiles();
    sendChangeMessage();
    return true;
}

// Deletes a profile file and clears Play view state if it was selected.
bool ProfileManager::deleteProfile(int profileIndex,
                                   juce::String* errorMessage) {
    const auto* profile = getProfile(profileIndex);
    if (profile == nullptr) {
        setError(errorMessage, "Profile not found.");
        return false;
    }

    const auto deletedProfileId = profile->id;

    if (!profile->file.deleteFile()) {
        setError(errorMessage, "Could not delete profile file: " + profile->file.getFullPathName());
        return false;
    }

    if (deletedProfileId == _currentProfileId) {
        clearCurrentProfile();
    }

    refreshProfiles();
    sendChangeMessage();
    return true;
}

// Imports a profile file into local storage with a fresh UUID identity.
bool ProfileManager::importProfile(const juce::File& sourceFile,
                                   juce::String* errorMessage) {
    if (!sourceFile.existsAsFile()) {
        setError(errorMessage, "Profile file does not exist.");
        return false;
    }

    auto importedProfile = readProfileFile(sourceFile, errorMessage);
    if (!importedProfile) {
        return false;
    }

    if (!ensureProfileDirectory(errorMessage)) {
        return false;
    }

    // Imported profiles get a fresh local UUID so they never overwrite a profile
    // that happens to share the same exported id.
    importedProfile->id = createUniqueProfileId();
    importedProfile->file = getFileForId(importedProfile->id);
    importedProfile->updatedAt = getTimestamp();
    importedProfile->values = normaliseProfileValues(importedProfile->values,
                                                     importedProfile->name);

    if (!writeProfileFile(*importedProfile, errorMessage)) {
        return false;
    }

    refreshProfiles();
    sendChangeMessage();
    return true;
}

// Applies APVTS-backed values from one loaded profile.
bool ProfileManager::applyProfile(int profileIndex,
                                  juce::String* errorMessage) {
    const auto* profile = getProfile(profileIndex);
    if (profile == nullptr) {
        setError(errorMessage, "Profile not found.");
        return false;
    }

    const auto& values = profile->values;

    // These ids are the UI/form field ids; each maps to the matching APVTS id.
    if (const auto* value = values.getVarPointer("masterVolume")) {
        setParameterValue("master", static_cast<float>(static_cast<double>(*value)));
    }

    if (const auto* value = values.getVarPointer("gain")) {
        setParameterValue("gain", static_cast<float>(static_cast<double>(*value)));
    }

    if (const auto* value = values.getVarPointer("noiseGate")) {
        setParameterValue("noise", static_cast<float>(static_cast<double>(*value)));
    }

    if (const auto* value = values.getVarPointer("bass")) {
        setParameterValue("bass", static_cast<float>(static_cast<double>(*value)));
    }

    if (const auto* value = values.getVarPointer("middle")) {
        setParameterValue("mid", static_cast<float>(static_cast<double>(*value)));
    }

    if (const auto* value = values.getVarPointer("treble")) {
        setParameterValue("treble", static_cast<float>(static_cast<double>(*value)));
    }

    if (const auto* value = values.getVarPointer("presence")) {
        setParameterValue("presence", static_cast<float>(static_cast<double>(*value)));
    }

    if (const auto* value = values.getVarPointer("depth")) {
        setParameterValue("depth", static_cast<float>(static_cast<double>(*value)));
    }

    return true;
}

// Applies the profile currently remembered by the Play view state file.
bool ProfileManager::applyCurrentProfile(juce::String* errorMessage) {
    const auto currentProfileIndex = getCurrentProfileIndex();

    if (currentProfileIndex < 0) {
        setError(errorMessage, "No current profile selected.");
        return false;
    }

    return applyProfile(currentProfileIndex, errorMessage);
}

// Applies a profile and then stores its UUID as the Play view current profile.
bool ProfileManager::applyProfileAndSaveAsCurrent(int profileIndex,
                                                  juce::String* errorMessage) {
    const auto* profile = getProfile(profileIndex);
    if (profile == nullptr) {
        setError(errorMessage, "Profile not found.");
        return false;
    }

    if (!applyProfile(profileIndex, errorMessage)) {
        return false;
    }

    setCurrentProfileId(profile->id);
    return true;
}

// Saves a loaded profile UUID as the Play view's current profile.
void ProfileManager::setCurrentProfileId(const juce::String& profileId) {
    const auto trimmedProfileId = profileId.trim();

    if (trimmedProfileId.isEmpty()) {
        clearCurrentProfile();
        return;
    }

    const auto previousProfileId = _currentProfileId;
    _currentProfileId = trimmedProfileId;

    // Only persist ids that still resolve to a loaded profile.
    if (getCurrentProfileIndex() < 0) {
        _currentProfileId = previousProfileId;
        return;
    }

    if (_currentProfileId != previousProfileId) {
        savePlayViewSettings();
        sendChangeMessage();
    }
}

// Clears the Play view current profile and persists the empty state.
void ProfileManager::clearCurrentProfile() {
    if (_currentProfileId.isEmpty()) {
        return;
    }

    _currentProfileId.clear();
    savePlayViewSettings();
    sendChangeMessage();
}

// Creates the profile directory when needed before reading or writing files.
bool ProfileManager::ensureProfileDirectory(juce::String* errorMessage) const {
    if (_profileDirectory.isDirectory()) {
        return true;
    }

    const auto result = _profileDirectory.createDirectory();
    if (result.wasOk()) {
        return true;
    }

    setError(errorMessage, "Could not create profile directory: " + result.getErrorMessage());
    return false;
}

// Writes one profile to disk using the `.profilerprofile` JSON schema.
bool ProfileManager::writeProfileFile(const Profile& profile,
                                      juce::String* errorMessage) const {
    auto rootObject = std::make_unique<juce::DynamicObject>();
    rootObject->setProperty(formatId, profileFormat);
    rootObject->setProperty(versionId, profileFormatVersion);
    rootObject->setProperty(idId, profile.id);
    rootObject->setProperty(nameId, profile.name);
    rootObject->setProperty(createdAtId, profile.createdAt);
    rootObject->setProperty(updatedAtId, profile.updatedAt);

    // Store profile values under one object so metadata can evolve independently.
    auto parametersObject = std::make_unique<juce::DynamicObject>();
    for (int i = 0; i < profile.values.size(); ++i) {
        parametersObject->setProperty(profile.values.getName(i), profile.values.getValueAt(i));
    }

    rootObject->setProperty(parametersId, juce::var(parametersObject.release()));

    const auto json = juce::JSON::toString(juce::var(rootObject.release()), false);
    if (profile.file.replaceWithText(json)) {
        return true;
    }

    setError(errorMessage, "Could not write profile file: " + profile.file.getFullPathName());
    return false;
}

// Reads and validates a profile JSON file from disk.
std::optional<Profile> ProfileManager::readProfileFile(const juce::File& file,
                                                               juce::String* errorMessage) const {
    auto parsed = juce::JSON::parse(file.loadFileAsString());
    auto* rootObject = parsed.getDynamicObject();

    if (rootObject == nullptr) {
        setError(errorMessage, "Invalid profile JSON: " + file.getFileName());
        return std::nullopt;
    }

    if (rootObject->getProperty(formatId).toString() != profileFormat) {
        setError(errorMessage, "Unsupported profile format: " + file.getFileName());
        return std::nullopt;
    }

    Profile profile;
    profile.file = file;
    profile.id = rootObject->getProperty(idId).toString();
    profile.name = rootObject->getProperty(nameId).toString();
    profile.createdAt = rootObject->getProperty(createdAtId).toString();
    profile.updatedAt = rootObject->getProperty(updatedAtId).toString();

    if (profile.id.isEmpty()) {
        profile.id = file.getFileNameWithoutExtension();
    }

    if (profile.name.isEmpty()) {
        profile.name = profile.id;
    }

    if (profile.createdAt.isEmpty()) {
        profile.createdAt = getTimestamp();
    }

    if (profile.updatedAt.isEmpty()) {
        profile.updatedAt = profile.createdAt;
    }

    // Older or hand-written files may omit parameters; missing fields simply keep
    // their current/default value when the profile is applied.
    const auto parameters = rootObject->getProperty(parametersId);
    if (auto* parametersObject = parameters.getDynamicObject()) {
        const auto& parameterProperties = parametersObject->getProperties();

        for (int i = 0; i < parameterProperties.size(); ++i) {
            profile.values.set(parameterProperties.getName(i), parameterProperties.getValueAt(i));
        }
    }

    profile.values = normaliseProfileValues(profile.values, profile.name);
    return profile;
}

// Builds the UUID-based profile filename inside the profile directory.
juce::File ProfileManager::getFileForId(const juce::String& profileId) const {
    return _profileDirectory.getChildFile(profileId + profileFileExtension);
}

// Chooses the default visible name for the next created profile.
juce::String ProfileManager::createDefaultProfileName() const {
    return "Profil " + juce::String(getProfileCount() + 1);
}

// Generates a UUID id that does not already have a matching file on disk.
juce::String ProfileManager::createUniqueProfileId() const {
    juce::String candidate;

    // UUID collisions are fantastically unlikely, but the file check is cheap.
    do {
        candidate = juce::Uuid().toDashedString();
    } while (getFileForId(candidate).existsAsFile());

    return candidate;
}

// Returns an ISO-8601 timestamp for profile metadata fields.
juce::String ProfileManager::getTimestamp() {
    return juce::Time::getCurrentTime().toISO8601(true);
}

// Extracts the user-facing profile name from form values.
juce::String ProfileManager::getProfileNameFromValues(const juce::NamedValueSet& values,
                                                      const juce::String& fallbackName) {
    auto name = fallbackName;

    if (const auto* value = values.getVarPointer(profileNameId)) {
        name = value->toString();
    }

    name = name.trim();
    return name.isNotEmpty() ? name : fallbackName;
}

// Ensures a values object includes the profileName field used by the forms.
juce::NamedValueSet ProfileManager::normaliseProfileValues(const juce::NamedValueSet& values,
                                                           const juce::String& profileName) {
    auto normalisedValues = values;
    normalisedValues.set(profileNameId, profileName);
    return normalisedValues;
}

// Stores a caller-visible error message when the caller requested one.
void ProfileManager::setError(juce::String* errorMessage, const juce::String& message) {
    if (errorMessage != nullptr) {
        *errorMessage = message;
    }
}

// Loads the Play view's last selected profile UUID from settings.
void ProfileManager::loadPlayViewSettings() {
    _currentProfileId.clear();

    const auto settingsFile = getPlayViewSettingsFile();
    if (!settingsFile.existsAsFile()) {
        return;
    }

    auto parsed = juce::JSON::parse(settingsFile.loadFileAsString());
    if (auto* settingsObject = parsed.getDynamicObject()) {
        _currentProfileId = settingsObject->getProperty(currentProfileId).toString().trim();
    }
}

// Saves the Play view's last selected profile UUID to settings.
void ProfileManager::savePlayViewSettings() const {
    auto settingsObject = std::make_unique<juce::DynamicObject>();
    settingsObject->setProperty(currentProfileId, _currentProfileId);

    const auto json = juce::JSON::toString(juce::var(settingsObject.release()), false);
    getPlayViewSettingsFile().replaceWithText(json);
}

// Applies a parameter value through APVTS using host notification.
void ProfileManager::setParameterValue(const juce::String& parameterId, float value) {
    if (auto* parameter = _apvts.getParameter(parameterId)) {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        parameter->endChangeGesture();
    }
}
