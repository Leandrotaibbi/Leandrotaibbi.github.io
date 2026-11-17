#pragma once

#include "PluginProcessor.h"

namespace polvere
{
namespace Colours
{
    const juce::Colour bg        { 0xff110f0e };
    const juce::Colour panel     { 0xff1b1716 };
    const juce::Colour edge      { 0xff2e2624 };
    const juce::Colour amber     { 0xffe2402f };   // display: rosso segnale
    const juce::Colour rose      { 0xfff1ece2 };   // display: bianco
    // pannello "desk": alluminio verniciato chiaro + serigrafia nera
    const juce::Colour paper     { 0xffe8e5de };
    const juce::Colour paperDark { 0xffd9d5cc };
    const juce::Colour ink       { 0xff161514 };
    const juce::Colour inkSoft   { 0xff5d5a55 };
    const juce::Colour signal    { 0xffd63a2a };
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
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override;
    juce::Font getLabelFont (juce::Label&) override;
};

//==============================================================================
// Controlli con MIDI learn (tasto destro)
using LearnCallback = std::function<void (const juce::String& paramId)>;

void drawLearnBadge (juce::Graphics&, juce::Component&, juce::Rectangle<int> area);

class LearnSlider : public juce::Slider
{
public:
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu() && onLearn) onLearn (paramId);
        else Slider::mouseDown (e);
    }
    void mouseDrag (const juce::MouseEvent& e) override { if (! e.mods.isPopupMenu()) Slider::mouseDrag (e); }
    void mouseUp (const juce::MouseEvent& e) override   { if (! e.mods.isPopupMenu()) Slider::mouseUp (e); }
    juce::String paramId;
    LearnCallback onLearn;
};

class LearnCombo : public juce::ComboBox
{
public:
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu() && onLearn) onLearn (paramId);
        else ComboBox::mouseDown (e);
    }
    void mouseUp (const juce::MouseEvent& e) override { if (! e.mods.isPopupMenu()) ComboBox::mouseUp (e); }
    juce::String paramId;
    LearnCallback onLearn;
};

class LearnButton : public juce::TextButton
{
public:
    explicit LearnButton (const juce::String& buttonText) : TextButton (buttonText) {}
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu() && onLearn && paramId.isNotEmpty()) onLearn (paramId);
        else if (! e.mods.isPopupMenu()) TextButton::mouseDown (e);
    }
    void mouseUp (const juce::MouseEvent& e) override { if (! e.mods.isPopupMenu()) TextButton::mouseUp (e); }
    void paintOverChildren (juce::Graphics& g) override { drawLearnBadge (g, *this, getLocalBounds()); }
    juce::String paramId;
    LearnCallback onLearn;
};

//==============================================================================
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& s, const juce::String& paramId, const juce::String& name, int modIndex = -1);
    void resized() override;
    void paintOverChildren (juce::Graphics& g) override { drawLearnBadge (g, *this, slider.getBounds()); }
    void setMod (float normalised);

    LearnSlider slider;
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
    void paintOverChildren (juce::Graphics& g) override { drawLearnBadge (g, *this, box.getBounds()); }

    juce::Label nameLabel;
    LearnCombo box;

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

class PatternView : public juce::Component
{
public:
    explicit PatternView (PolvereProcessor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
    float flash = 0.0f;

private:
    PolvereProcessor& proc;
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
    void layoutSection (Section&, juce::Rectangle<int> area, int knobW = 70);
    Section& addSection (const juce::String& title, std::initializer_list<juce::Component*> cells,
                         const juce::StringArray& params, bool withDice = true);
    Section& section (const juce::String& title);
    void showLearnMenu (const juce::String& paramId);
    void setupButton (polvere::LearnButton& b, const juce::String& paramId);
    void openLoader();
    float s_diceAmount() const;
    void updateStatus();

    PolvereProcessor& proc;
    polvere::LookAndFeel laf;
    polvere::GrainCloud cloud;
    polvere::PatternView patternView;

    std::vector<std::unique_ptr<polvere::Knob>> knobs;
    std::vector<std::unique_ptr<polvere::Choice>> choices;
    std::vector<std::unique_ptr<Section>> sections;

    juce::TextButton loadButton { "LOAD" }, clearButton { "CLEAR" };
    polvere::LearnButton playButton { "PLAY" }, generateButton { "GENERATE" }, diceAllButton { "DICE ALL" },
                         undoButton { "UNDO" }, droneButton { "DRONE" }, keyButton { "KEY TRACK" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> playAtt, droneAtt, keyAtt;
    juce::Label statusLabel;

    std::unique_ptr<juce::FileChooser> chooser;
    double lastTick = 0.0;
    uint32_t lastPatternVersion = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolvereEditor)
};
