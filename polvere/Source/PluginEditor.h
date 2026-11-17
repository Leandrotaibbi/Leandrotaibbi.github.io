#pragma once

#include "PluginProcessor.h"

namespace polvere
{
namespace Colours
{
    const juce::Colour bg        { 0xff110f0e };
    const juce::Colour panel     { 0xff1b1716 };
    const juce::Colour edge      { 0xff2e2624 };
    const juce::Colour amber     { 0xffe9a55b };
    const juce::Colour rose      { 0xffd9677f };
    const juce::Colour cream     { 0xffeadfcc };
    const juce::Colour dim       { 0xff86786f };
    const juce::Colour track     { 0xff3a302d };
}

//==============================================================================
class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override;
    juce::Font getLabelFont (juce::Label&) override;
};

//==============================================================================
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& s, const juce::String& paramId, const juce::String& name, int modIndex = -1);
    void resized() override;
    void setMod (float normalised);

    juce::Slider slider;
    juce::Label nameLabel, valueLabel;
    int modIndex;

private:
    void refreshValue();
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::RangedAudioParameter* param;
    float lastMod = -2.0f;
};

class Choice : public juce::Component
{
public:
    Choice (juce::AudioProcessorValueTreeState& s, const juce::String& paramId, const juce::String& name);
    void resized() override;

    juce::Label nameLabel;
    juce::ComboBox box;

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

//==============================================================================
class GrainCloud : public juce::Component
{
public:
    explicit GrainCloud (PolvereProcessor& p) : proc (p) {}

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void addEvent (const GrainEvent& e);
    void tick (float dt);

    bool dragOver = false;

private:
    struct Particle { int lane; float x, y, pan, age, life; bool rev; };
    int laneAt (float y) const;
    juce::Rectangle<float> laneBounds (int lane, int numLanes) const;

    PolvereProcessor& proc;
    std::vector<Particle> particles;
};

} // namespace polvere

//==============================================================================
class PolvereEditor : public juce::AudioProcessorEditor,
                      public juce::FileDragAndDropTarget,
                      private juce::Timer,
                      private juce::ChangeListener
{
public:
    explicit PolvereEditor (PolvereProcessor&);
    ~PolvereEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void fileDragEnter (const juce::StringArray&, int, int) override;
    void fileDragExit (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray& files, int, int) override;

private:
    struct Section
    {
        juce::String title;
        juce::StringArray params;
        std::vector<juce::Component*> cells;
        std::unique_ptr<juce::TextButton> dice;
        juce::Rectangle<int> bounds;
    };

    void timerCallback() override;
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void layoutSection (Section&, juce::Rectangle<int> area);
    Section& addSection (const juce::String& title, std::initializer_list<juce::Component*> cells, const juce::StringArray& params);
    void dice (const juce::StringArray& ids, float amount = 1.0f);
    void undo();
    void openLoader();
    void updateStatus();
    juce::StringArray allDiceable() const;

    PolvereProcessor& proc;
    polvere::LookAndFeel laf;
    polvere::GrainCloud cloud;
    juce::MidiKeyboardComponent keyboard;

    std::vector<std::unique_ptr<polvere::Knob>> knobs;
    std::vector<std::unique_ptr<polvere::Choice>> choices;
    std::vector<std::unique_ptr<Section>> sections;

    juce::TextButton loadButton { "LOAD" }, clearButton { "CLEAR" }, diceAllButton { "DICE ALL" },
                     undoButton { "UNDO" }, droneButton { "DRONE" }, keyButton { "KEY TRACK" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> droneAtt, keyAtt;
    juce::Label statusLabel;
    juce::Slider chaosSlider;
    juce::Label chaosLabel;

    std::vector<std::vector<float>> undoStack;
    juce::Random random;
    std::unique_ptr<juce::FileChooser> chooser;
    double lastTick = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolvereEditor)
};
