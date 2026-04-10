#include "AmpProfiling.h"

void AmpProfiling::generateAndSaveGainSignal() {
    double sampleRate = 44100.0;
    int numSamples = (int)(2.0 * sampleRate);
    juce::AudioBuffer<float> buffer(1, numSamples);

    float freq = 1000.0f;
    float amp = 0.25f;  // On monte à -12dB pour mieux voir la courbe

    for (int i = 0; i < numSamples; ++i) {
        float angle = 2.0f * juce::MathConstants<float>::pi * freq *
                      (i / (float)sampleRate);
        buffer.setSample(0, i, std::sin(angle) * amp);
    }

    buffer.applyGainRamp(0, 0, 1000, 0.0f, 1.0f);
    buffer.applyGainRamp(0, numSamples - 1000, 1000, 1.0f, 0.0f);

    _chooser = std::make_unique<juce::FileChooser>(
        "Sauver",
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
        "*.wav");

    _chooser->launchAsync(
        juce::FileBrowserComponent::saveMode,
        [buffer, sampleRate](const juce::FileChooser& fc) mutable {
            auto file = fc.getResult();
            if (file != juce::File()) {
                file = file.withFileExtension(".wav");
                juce::WavAudioFormat wavFormat;
                if (auto writer = wavFormat.createWriterFor(
                        file.createOutputStream().release(), sampleRate, 1, 24,
                        {}, 0)) {
                    writer->writeFromAudioSampleBuffer(buffer, 0,
                                                       buffer.getNumSamples());
                    delete writer;  // <--- LE FIX EST ICI
                }
            }
        });
}

void AmpProfiling::startGainAnalysis(AmpProcessor& processor) {
    _chooser = std::make_unique<juce::FileChooser>(
        "1. Sélectionne l'Original (Avant)",
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
        "*.wav");

    _chooser->launchAsync(
        juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles,
        [this, &processor](const juce::FileChooser& fc1) {
            auto fileRef = fc1.getResult();
            if (fileRef == juce::File()) return;

            _chooser = std::make_unique<juce::FileChooser>(
                "2. Sélectionne l'Enregistré (Après)",
                fileRef.getParentDirectory(), "*.wav");

            _chooser->launchAsync(
                juce::FileBrowserComponent::openMode |
                    juce::FileBrowserComponent::canSelectFiles,
                [this, &processor, fileRef](const juce::FileChooser& fc2) {
                    auto fileRec = fc2.getResult();
                    if (fileRec == juce::File()) return;

                    juce::AudioFormatManager formatManager;
                    formatManager.registerBasicFormats();

                    std::unique_ptr<juce::AudioFormatReader> readerRef(
                        formatManager.createReaderFor(fileRef));
                    std::unique_ptr<juce::AudioFormatReader> readerRec(
                        formatManager.createReaderFor(fileRec));

                    if (readerRef != nullptr && readerRec != nullptr) {
                        int numSamplesRef = (int)readerRef->lengthInSamples;
                        int numSamplesRec = (int)readerRec->lengthInSamples;

                        juce::AudioBuffer<float> bufferRef(1, numSamplesRef);
                        juce::AudioBuffer<float> bufferRec(1, numSamplesRec);

                        readerRef->read(&bufferRef, 0, numSamplesRef, 0, true,
                                        false);
                        readerRec->read(&bufferRec, 0, numSamplesRec, 0, true,
                                        false);

                        // --- ALIGNEMENT ---
                        int bestDelay = 0;
                        float maxCorr = -1.0f;
                        int searchRange = (int)readerRef->sampleRate;
                        int windowSize = 2000;

                        for (int d = 0; d < searchRange; ++d) {
                            float corr = 0;
                            for (int i = 0; i < windowSize; ++i) {
                                if (i + d < numSamplesRec)
                                    corr += bufferRef.getSample(0, i) *
                                            bufferRec.getSample(0, i + d);
                            }
                            if (corr > maxCorr) {
                                maxCorr = corr;
                                bestDelay = d;
                            }
                        }

                        // --- CALCUL DU GAIN A ---
                        int analysisLength = (int)readerRef->sampleRate;
                        if (bestDelay + analysisLength > numSamplesRec)
                            analysisLength = numSamplesRec - bestDelay;

                        float rmsRec =
                            bufferRec.getRMSLevel(0, bestDelay, analysisLength);

                        // On applique rmsRec directement à A
                        // car la LUT sort un signal "normalisé" proche de 1.0.
                        processor.setA(rmsRec);

                        juce::Logger::writeToLog(
                            "Test Gain Statique : A réglé sur " +
                            juce::String(rmsRec));
                    }
                });
        });
}

// Pour garder le code propre, on déplace le calcul dans une fonction séparée
void AmpProfiling::processGainAnalysis(juce::File fileRef, juce::File fileRec,
                                       AmpProcessor& processor) {
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> readerRef(
        formatManager.createReaderFor(fileRef));
    std::unique_ptr<juce::AudioFormatReader> readerRec(
        formatManager.createReaderFor(fileRec));

    if (readerRef != nullptr && readerRec != nullptr) {
        // ... ICI LE RESTE DE TON CODE (Corrélation, RMS, etc.) ...
        // Tu utilises readerRef et readerRec directement.

        juce::Logger::writeToLog(
            "Analyse lancée entre : " + fileRef.getFileName() + " et " +
            fileRec.getFileName());

        // (Copie ici la logique de buffers et de boucles de ton code original)
    }
}

// LUT

#include "AmpProfiling.h"

// 1. GENERER LE SIGNAL DE RAMPE (PROBE)
void AmpProfiling::generateSaturationProbe() {
    double sampleRate = 44100.0;
    // Une rampe très lente pour éviter que les filtres de l'ampli ne déphasent
    // trop
    int numSamples = (int)(5.0 * sampleRate);
    juce::AudioBuffer<float> buffer(1, numSamples);

    for (int i = 0; i < numSamples; ++i) {
        // Génère une ligne droite de -1.0 à +1.0
        float val = (2.0f * i / (float)(numSamples - 1)) - 1.0f;
        buffer.setSample(0, i, val);
    }

    _chooser = std::make_unique<juce::FileChooser>(
        "Sauver le Probe de Saturation",
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
        "*.wav");

    _chooser->launchAsync(
        juce::FileBrowserComponent::saveMode,
        [buffer, sampleRate](const juce::FileChooser& fc) mutable {
            auto file = fc.getResult();
            if (file != juce::File()) {
                file = file.withFileExtension(".wav");
                juce::WavAudioFormat wavFormat;
                if (auto writer = wavFormat.createWriterFor(
                        file.createOutputStream().release(), sampleRate, 1, 24,
                        {}, 0)) {
                    writer->writeFromAudioSampleBuffer(buffer, 0,
                                                       buffer.getNumSamples());
                    delete writer;
                }
            }
        });
}

// 2. ANALYSER ET CLONER LA LUT
void AmpProfiling::startSaturationAnalysis(AmpProcessor& processor) {
    _chooser = std::make_unique<juce::FileChooser>(
        "Sélectionne le WET (Enregistré)",
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
        "*.wav");

    _chooser->launchAsync(
        juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles,
        [this, &processor](const juce::FileChooser& fc) {
            auto fileRec = fc.getResult();
            if (fileRec == juce::File()) return;

            juce::AudioFormatManager formatManager;
            formatManager.registerBasicFormats();
            std::unique_ptr<juce::AudioFormatReader> readerRec(
                formatManager.createReaderFor(fileRec));

            if (readerRec != nullptr) {
                int numSamples = (int)readerRec->lengthInSamples;
                juce::AudioBuffer<float> bufferRec(1, numSamples);
                readerRec->read(&bufferRec, 0, numSamples, 0, true, false);

                // On crée une table temporaire pour stocker la capture
                std::vector<float> capturedLut(processor.getLutSize());

                /* LOGIQUE DE DEDUCTION :
                   Puisque le RAW était une rampe parfaite de -1 à 1,
                   chaque échantillon du WET correspond DIRECTEMENT à un index
                   de la LUT.
                */
                for (int i = 0; i < processor.getLutSize(); ++i) {
                    // On mappe l'index de la LUT sur la position dans le
                    // fichier audio
                    float progress =
                        (float)i / (float)(processor.getLutSize() - 1);
                    int sampleIdx = (int)(progress * (numSamples - 1));

                    // On récupère la valeur saturée
                    capturedLut[i] = bufferRec.getSample(0, sampleIdx);
                }

                // On envoie la nouvelle courbe au processeur
                processor.updateLUT(capturedLut);
                juce::Logger::writeToLog("Saturation clonée avec succès !");
            }
        });
}