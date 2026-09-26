#pragma once

#include <JuceHeader.h>

#include <memory>

class SpectraEngine {
   public:
    SpectraEngine();
    ~SpectraEngine();

    bool open(const juce::File& libraryFile = {});
    void close();
    bool isLoaded() const noexcept;
    int abiVersion() const noexcept;

    bool unlock(const juce::String& accessToken);
    void lock();

    bool decryptModel(const void* cipher, size_t size, juce::MemoryBlock& plain) const;
    bool decryptIr(const void* cipher, size_t size, juce::MemoryBlock& plain) const;

   private:
    struct Functions;

    std::unique_ptr<Functions> _functions;
    juce::DynamicLibrary _library;
    int _abiVersion = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectraEngine)
};
