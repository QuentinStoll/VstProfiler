#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "SpectraEngine.h"

namespace {
juce::MemoryBlock makeSilentWav() {
    std::unique_ptr<juce::OutputStream> stream = std::make_unique<juce::MemoryOutputStream>();
    auto* rawStream = static_cast<juce::MemoryOutputStream*>(stream.get());
    juce::WavAudioFormat format;
    auto writer = format.createWriterFor(stream, juce::AudioFormatWriterOptions{}.withSampleRate(44100.0));
    if (writer == nullptr) {
        return {};
    }

    juce::AudioBuffer<float> samples(1, 8);
    samples.clear();
    const auto wrote = writer->writeFromAudioSampleBuffer(samples, 0, samples.getNumSamples());
    writer->flush();
    auto block = rawStream->getMemoryBlock();
    writer.reset();
    if (!wrote) {
        return {};
    }
    return block;
}
}  // namespace

class SpectraEngineTests : public juce::UnitTest {
   public:
    SpectraEngineTests()
        : juce::UnitTest("SpectraEngine", "Profiler") {}

    void runTest() override {
        beginTest("spectra engine decrypts only after unlock");
        SpectraEngine engine;
        const auto loaded = engine.open();
        const char cipher[] = {'a', 'm', 'p'};
        juce::MemoryBlock plain;

        if (!loaded) {
            expect(!engine.isLoaded(), "Missing library stays unloaded.");
            expect(!engine.decryptModel(cipher, sizeof(cipher), plain), "Decrypt fails without the library.");
            expect(plain.isEmpty(), "No plaintext is produced without the library.");
        } else {
            expectEquals(engine.abiVersion(), 1);
            expect(!engine.unlock({}), "Empty token is rejected.");
            expect(!engine.decryptModel(cipher, sizeof(cipher), plain), "Decrypt waits for a session.");
            expect(engine.unlock("session"), "Session unlocks.");
            expect(engine.decryptModel(cipher, sizeof(cipher), plain), "Model decrypts in memory.");
            expect(plain.getSize() > 0, "Decrypt returns a buffer.");
            engine.lock();
            expect(!engine.decryptIr(cipher, sizeof(cipher), plain), "Lock closes decryption.");
        }

        beginTest("protected assets load from memory");
        ProfilerAudioProcessor processor;
        const char junk[] = {'n', 'o', 'p', 'e'};
        expect(!processor.loadAmpFromMemory(nullptr, 0), "Empty model is rejected.");
        expect(!processor.loadAmpFromMemory(junk, sizeof(junk)), "Invalid model JSON is rejected.");
        expect(!processor.isAmpFileLoaded(), "A rejected model does not publish a file.");
        expect(!processor.loadIrFromMemory(nullptr, 0), "Empty IR is rejected.");

        const auto wav = makeSilentWav();
        expect(wav.getSize() > 0, "Test wav should be written.");
        expect(processor.loadIrFromMemory(wav.getData(), wav.getSize()), "IR loads from memory.");
        expect(processor.isIRLoaded(), "In-memory IR is active.");
        expect(processor.getCurrentIRFile() == juce::File{}, "In-memory IR has no plaintext path.");

        if (!processor.isSpectraLoaded()) {
            expect(!processor.loadProtectedAmp(junk, sizeof(junk)), "Protected amp needs the library.");
            expect(!processor.loadProtectedIr(wav.getData(), wav.getSize()), "Protected IR needs the library.");
            return;
        }

        expect(!processor.loadProtectedAmp(junk, sizeof(junk)), "Protected amp waits for a session.");
        expect(processor.unlockSpectraSession("session"), "Processor session unlocks.");
        expect(!processor.loadProtectedAmp(junk, sizeof(junk)), "Protected junk is not a model.");
        expect(!processor.isAmpFileLoaded(), "Protected load does not invent a file path.");
        expect(processor.loadProtectedIr(wav.getData(), wav.getSize()), "Protected IR decrypts into the convolver.");
        expect(processor.isIRLoaded(), "Protected IR is active.");
        expect(processor.getCurrentIRFile() == juce::File{}, "Protected IR is not stored as a wav path.");
        processor.lockSpectraSession();
    }
};

static SpectraEngineTests spectraEngineTests;
