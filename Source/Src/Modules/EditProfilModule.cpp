#include "Modules/EditProfilModule.h"

void EditProfilModule::setProfileNumber(int profileNumber) {
    _profileNumber = profileNumber;
}

int EditProfilModule::getProfileNumber() const {
    return _profileNumber;
}

void EditProfilModule::paint(juce::Graphics& /*g*/) {
}
