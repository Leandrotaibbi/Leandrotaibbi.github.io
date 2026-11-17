#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "GrainEngine.h"

class PolvereProcessor : public juce::AudioProcessor,
                         public juce::ChangeBroadcaster,
                         private juce::AsyncUpdater,
                         private juce::Timer
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
    // Randomizzazione (message thread) — chiamabile dalla GUI o dai parametri trigger
    void randomise (const juce::StringArray& ids, float amount);
    void undoRandomise();
    juce::StringArray getDiceableIds() const;
    void requestGenerate() noexcept { generateRequested.store (true); }

    // MIDI learn (message thread)
    void startMidiLearn (const juce::String& paramId);
    void cancelMidiLearn();
    void clearMidiMapping (const juce::String& paramId);
    int getCCForParam (const juce::String& paramId) const;
    juce::String getLearningParam() const;

    //==============================================================================
    // Dati per la GUI (lock-free)
    juce::AbstractFifo eventFifo { 1024 };
    std::array<polvere::GrainEvent, 1024> eventBuffer;
    std::atomic<int> activeGrainCount { 0 };
    std::atomic<int> activeVoiceCount { 0 };
    std::atomic<float> scanPhaseDisplay { 0.0f };
    std::atomic<int> currentStep { -1 };
    std::atomic<bool> stutterActive { false };
    std::atomic<uint32_t> patternVersion { 0 };
    std::atomic<int> rootNote { 60 };
    std::array<polvere::Step, polvere::maxSteps> pattern {};   // scritto dall'audio thread

    enum ModTarget { modPos = 0, modSize, modDensity, modPitchRnd, modCutoff, modSpray, numModTargets };
    std::array<std::atomic<float>, numModTargets> modNorm;

    juce::AudioProcessorValueTreeState apvts;

    static constexpr int maxGrains = 512;
    static constexpr int maxVoices = 12;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void handleAsyncUpdate() override;
    void timerCallback() override;
    void publishPool();

    void handleMidi (const juce::MidiMessage& m);
    int noteOn (int note, float velocity, float posOffset = 0.0f, float sizeMul = 1.0f);
    void noteOffVoice (int voice, uint32_t age);
    void releaseSequencerVoices();
    void spawnGrain (int voiceIndex, int offset);
    void renderGrains (float* outL, float* outR, int numSamples);
    float quantise (float semis, int scale) const noexcept;

    // sequencer generativo
    void generateStep (int i) noexcept;
    void generatePattern() noexcept;
    void triggerStep (juce::int64 absStep, int offset, double stepSamples);
    void scheduleEvent (const polvere::SeqEvent& e) noexcept;
    void updateChaos (float secs, bool stepTick) noexcept;

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

    // pattern caricato dallo stato (message thread -> audio thread)
    juce::SpinLock patternLock;
    std::array<polvere::Step, polvere::maxSteps> pendingPattern {};
    bool hasPendingPattern = false;

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
    std::array<float, numModTargets> chaosOut {};
    std::array<float, numModTargets> logistic {};
    float lorX = 0.1f, lorY = 0.0f, lorZ = 20.0f;
    float shPhase = 0.0f;
    double scanPhase = 0.0;
    bool droneWasOn = false;

    // sequencer
    std::array<polvere::SeqEvent, 128> events {};
    double freePpq = 0.0;
    juce::int64 lastAbsStep = -1;
    bool wasPlaying = false;
    std::atomic<bool> generateRequested { false };

    // stutter / crush
    juce::AudioBuffer<float> stutBuf;
    int stutWrite = 0, stutStart = 0, stutLen = 0, stutPhase = 0, stutRemaining = 0;
    float crushHoldL = 0, crushHoldR = 0;
    int crushCounter = 0;

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

    // trigger da parametri (message thread)
    bool lastGenerate = false, lastDice = false, lastUndo = false;
    std::vector<std::vector<float>> undoStack;
    juce::Random msgRandom;

    // MIDI learn
    std::array<std::atomic<int>, 128> ccMap;         // cc -> indice parametro
    std::array<std::atomic<float>, 128> pendingCC;   // valore da applicare (-1 = niente)
    std::atomic<int> learnParam { -1 };
    juce::Array<juce::AudioProcessorParameter*> paramList;
    int indexOfParam (const juce::String& id) const;

    juce::RangedAudioParameter *rpSize = nullptr, *rpDensity = nullptr, *rpPitchRnd = nullptr, *rpCutoff = nullptr;

    std::atomic<float>* p (const char* id) { return apvts.getRawParameterValue (id); }
    std::atomic<float> *pPos, *pSpray, *pScan, *pSize, *pSizeRnd, *pDensity, *pDensRnd, *pChance,
                       *pPitch, *pPitchRnd, *pScale, *pDetune, *pReverse, *pKeytrack,
                       *pWindow, *pPan, *pSample, *pSampleRnd, *pCutoff, *pCutRnd, *pReso, *pFtype,
                       *pMutate, *pMutRate, *pModType, *pDrone,
                       *pAttack, *pDecay, *pSustain, *pRelease,
                       *pPlay, *pSteps, *pRate, *pFill, *pRange, *pRatchet, *pEvolve, *pPlock, *pGate, *pSwing,
                       *pStutter, *pStutLen, *pCrush,
                       *pDlyTime, *pDlyFb, *pDlyMix, *pVerbSize, *pVerbMix, *pWidth, *pGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolvereProcessor)
};
