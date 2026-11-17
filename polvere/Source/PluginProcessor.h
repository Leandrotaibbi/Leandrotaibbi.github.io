#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "GrainEngine.h"

class PolvereProcessor : public juce::AudioProcessor,
                         public juce::ChangeBroadcaster,
                         private juce::AsyncUpdater
{
public:
    PolvereProcessor();
    ~PolvereProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Polvere"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Pool dei sample (message thread)
    void loadFiles (const juce::Array<juce::File>& files, bool replaceExisting);
    void clearSamples();
    void removeSample (int index);
    const polvere::SamplePool& getUiPool() const noexcept { return uiPool; }
    bool isLoading() const noexcept { return pendingJobs.load() > 0; }

    static juce::String getSupportedWildcard() { return "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg;*.caf;*.m4a"; }

    //==============================================================================
    // Dati per la GUI (lock-free)
    juce::AbstractFifo eventFifo { 1024 };
    std::array<polvere::GrainEvent, 1024> eventBuffer;
    std::atomic<int> activeGrainCount { 0 };
    std::atomic<int> activeVoiceCount { 0 };
    std::atomic<float> scanPhaseDisplay { 0.0f };
    enum ModTarget { modPos = 0, modSize, modDensity, modPitchRnd, modCutoff, modSpray, numModTargets };
    std::array<std::atomic<float>, numModTargets> modNorm;

    juce::MidiKeyboardState keyboardState;
    juce::AudioProcessorValueTreeState apvts;

    static constexpr int maxGrains = 512;
    static constexpr int maxVoices = 8;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void handleAsyncUpdate() override;
    void publishPool();

    void handleMidi (const juce::MidiMessage& m);
    void noteOn (int note, float velocity);
    void noteOff (int note);
    void spawnGrain (int voiceIndex, int offset);
    void renderGrains (float* outL, float* outR, int numSamples);
    float quantise (float semis, int scale) const noexcept;

    //==============================================================================
    juce::AudioFormatManager formatManager;
    juce::ThreadPool loaderPool { 1 };
    std::atomic<int> pendingJobs { 0 };
    juce::CriticalSection loadedLock;
    polvere::SamplePool loadedQueue;
    bool replaceOnNextMerge = false;

    polvere::SamplePool uiPool;      // message thread
    juce::SpinLock poolLock;
    polvere::SamplePool pendingPool; // handoff
    bool hasPendingPool = false;
    polvere::SamplePool activePool;  // audio thread

    //==============================================================================
    double hostRate = 44100.0;
    std::array<polvere::Grain, maxGrains> grains;
    std::array<polvere::Voice, maxVoices> voices;
    uint32_t voiceCounter = 0;
    static constexpr int subBlock = 32;
    std::array<std::array<float, subBlock>, maxVoices> envBuf {};
    polvere::FastRandom rng;
    polvere::WindowTables windows;
    std::array<polvere::Wander, numModTargets> wander;
    double scanPhase = 0.0;
    bool droneWasOn = false;

    // parametri effettivi del sub-blocco corrente
    struct Frame
    {
        float pos, spray, sizeMs, sizeRnd, density, densRnd, chance;
        float pitch, pitchRnd, detune, reverse;
        int scale, window, ftype;
        bool keytrack;
        float pan, sample, sampleRnd, cutoff, cutRnd, reso;
    } frame {};

    // FX
    juce::AudioBuffer<float> delayBuf;
    int delayWrite = 0;
    juce::SmoothedValue<float> delaySmoothed;
    float dampL = 0, dampR = 0;
    juce::Reverb reverb;
    juce::SmoothedValue<float> gainSmoothed;

    juce::RangedAudioParameter *rpSize = nullptr, *rpDensity = nullptr, *rpPitchRnd = nullptr, *rpCutoff = nullptr;

    // puntatori raw ai parametri
    std::atomic<float>* p (const char* id) { return apvts.getRawParameterValue (id); }
    std::atomic<float> *pPos, *pSpray, *pScan, *pSize, *pSizeRnd, *pDensity, *pDensRnd, *pChance,
                       *pPitch, *pPitchRnd, *pScale, *pDetune, *pReverse, *pKeytrack,
                       *pWindow, *pPan, *pSample, *pSampleRnd, *pCutoff, *pCutRnd, *pReso, *pFtype,
                       *pMutate, *pMutRate, *pDrone,
                       *pAttack, *pDecay, *pSustain, *pRelease,
                       *pDlyTime, *pDlyFb, *pDlyMix, *pVerbSize, *pVerbMix, *pWidth, *pGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolvereProcessor)
};
