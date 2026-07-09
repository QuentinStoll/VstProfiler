#pragma once

#include <JuceHeader.h>

#define RTNEURAL_DEFAULT_STATIC 1
#define RTNEURAL_ENABLE_LSTM 1
#define RTNEURAL_ENABLE_GRU 1
#define RTNEURAL_ENABLE_DENSE 1

#include <RTNeural/RTNeural.h>

#include "ProfileManager.h"

//==============================================================================
/**
 */
class ProfilerAudioProcessor : public juce::AudioProcessor {
   public:
    //==============================================================================
    ProfilerAudioProcessor();
    ~ProfilerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState _apvts{*this, nullptr, "Parameters", createParameterLayout()};

    //===================================== Our func ===============================

    // Loading the Impulse responce file
    void loadIRFile();
    bool loadIRFile(const juce::File& file);
    void unloadIRFile();
    bool loadAmpFile(const juce::File& file);
    void unloadAmpFile();
    bool isIRLoaded() const noexcept;
    bool isAmpFileLoaded() const noexcept;
    juce::File getCurrentIRFile() const;
    juce::File getCurrentAmpFile() const;
    bool applyProfile(int profileIndex, juce::String* errorMessage = nullptr);
    void syncLoadedFilesWithCurrentProfile();
    juce::String getAppliedProfileId() const;
    void clearAppliedProfile();
    ProfileManager& getProfileManager() noexcept;
    const ProfileManager& getProfileManager() const noexcept;

   private:
    enum ChainPositions {
        Gain = 0,
        NoiseGate,
        MasterVolume,
        Depth,
        Bass,
        Mid,
        Treble,
        Presence
    };

    using Filter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    using Chain = juce::dsp::ProcessorChain<
        juce::dsp::Gain<float>,
        juce::dsp::NoiseGate<float>,
        juce::dsp::Gain<float>,
        Filter,
        Filter,
        Filter,
        Filter,
        Filter>;

    Chain _chain;

    static constexpr float DEPTH_FREQ{60.0f};
    static constexpr float BASS_FREQ{200.0f};
    static constexpr float MID_FREQ{800.0f};
    static constexpr float TREBLE_FREQ{3200.0f};
    static constexpr float PRESENCE_FREQ{8000.0f};
    static constexpr float SHELF_Q{0.707f};
    static constexpr float PEAK_Q{1.0f};

    std::atomic<float>* _masterParam{nullptr};
    std::atomic<float>* _gainParam{nullptr};
    std::atomic<float>* _noiseParam{nullptr};

    std::atomic<float>* _depthParam{nullptr};
    std::atomic<float>* _bassParam{nullptr};
    std::atomic<float>* _midParam{nullptr};
    std::atomic<float>* _trebleParam{nullptr};
    std::atomic<float>* _presenceParam{nullptr};

    std::atomic<float>* _isMuteParam{nullptr};
    std::atomic<float>* _isEqEnabledParam{nullptr};

    ProfileManager _profileManager;
    juce::String _appliedProfileId;

    void updateEqCoefficients();
    void applyProfileFileValues(const juce::NamedValueSet& values);

    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProfilerAudioProcessor)

    //================================= Ir load =====================================

    // Buffer that contain the ir
    juce::AudioBuffer<float> _irBuffer;

    // Is ir loaded bool
    bool _irLoaded = false;
    juce::File _currentIRFile;

    // Convolver object
    juce::dsp::Convolution _convolver;

    //================================= Amp file load ====================================
    bool _ampFileLoaded = false;
    juce::File _currentAmpFile;

    //================================= RTNeural Load ====================================
    // Declaration of the model type (for example, a generic sequential model)
    // You can adjust the type according to your model architecture (LSTM, Dense, Conv, etc.)
    std::unique_ptr<RTNeural::Model<float>> _neuralAmp;

    juce::CriticalSection _ampModelLock;
    bool _ampLoaded = false;  // Initialized to false until the JSON is loaded

    // Keep the oversampler if needed, but be careful with the model's training sample rate!
    juce::dsp::Oversampling<float> oversampler{2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true};

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> _dcBlocker;
};
