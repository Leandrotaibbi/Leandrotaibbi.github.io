#include "PluginEditor.h"

using namespace polvere;

//==============================================================================
// LOOK AND FEEL
//==============================================================================
LookAndFeel::LookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Colours::bg);
    setColour (juce::Label::textColourId, Colours::cream);
    setColour (juce::ComboBox::backgroundColourId, Colours::panel);
    setColour (juce::ComboBox::textColourId, Colours::cream);
    setColour (juce::ComboBox::outlineColourId, Colours::edge);
    setColour (juce::ComboBox::arrowColourId, Colours::amber);
    setColour (juce::PopupMenu::backgroundColourId, Colours::panel);
    setColour (juce::PopupMenu::textColourId, Colours::cream);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours::amber);
    setColour (juce::PopupMenu::highlightedTextColourId, Colours::bg);
    setColour (juce::TextButton::buttonColourId, Colours::panel);
    setColour (juce::TextButton::buttonOnColourId, Colours::amber);
    setColour (juce::TextButton::textColourOffId, Colours::cream);
    setColour (juce::TextButton::textColourOnId, Colours::bg);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                    float a0, float a1, juce::Slider& s)
{
    const auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.0f);
    const float r = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto c = bounds.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    const float arcR = r - 2.0f;

    // corpo
    g.setColour (Colours::panel.brighter (0.06f));
    g.fillEllipse (c.x - r * 0.72f, c.y - r * 0.72f, r * 1.44f, r * 1.44f);
    g.setColour (Colours::edge.brighter (0.2f));
    g.drawEllipse (c.x - r * 0.72f, c.y - r * 0.72f, r * 1.44f, r * 1.44f, 1.0f);

    // traccia
    juce::Path tr;
    tr.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, a0, a1, true);
    g.setColour (Colours::track);
    g.strokePath (tr, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // valore
    const bool bipolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;
    const float from = bipolar ? (a0 + a1) * 0.5f : a0;
    juce::Path va;
    va.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, juce::jmin (from, ang), juce::jmax (from, ang), true);
    g.setColour (s.isEnabled() ? Colours::amber : Colours::dim);
    g.strokePath (va, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // modulazione "mutate"
    const float mod = (float) s.getProperties().getWithDefault ("mod", -1.0f);
    if (mod >= 0.0f)
    {
        const float ma = a0 + juce::jlimit (0.0f, 1.0f, mod) * (a1 - a0);
        juce::Path ma1;
        const float mr = arcR + 4.0f;
        ma1.addCentredArc (c.x, c.y, mr, mr, 0.0f, juce::jmin (ang, ma), juce::jmax (ang, ma), true);
        g.setColour (Colours::rose.withAlpha (0.8f));
        g.strokePath (ma1, juce::PathStrokeType (1.5f));
        const auto dot = c.getPointOnCircumference (mr, ma);
        g.fillEllipse (dot.x - 2.5f, dot.y - 2.5f, 5.0f, 5.0f);
    }

    // puntatore
    const auto p1 = c.getPointOnCircumference (r * 0.25f, ang);
    const auto p2 = c.getPointOnCircumference (r * 0.62f, ang);
    g.setColour (Colours::cream);
    g.drawLine ({ p1, p2 }, 2.0f);
}

void LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();
    const auto accent = b.getProperties().getWithDefault ("rose", false) ? Colours::rose : Colours::amber;
    g.setColour (on ? accent : Colours::panel.brighter (over ? 0.08f : 0.0f).darker (down ? 0.2f : 0.0f));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (on ? accent : (over ? accent.withAlpha (0.7f) : Colours::edge.brighter (0.3f)));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);
}

void LookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    g.setFont (getTextButtonFont (b, b.getHeight()));
    g.setColour (b.getToggleState() ? Colours::bg : Colours::cream);
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred, false);
}

void LookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
    g.setColour (Colours::panel.brighter (0.05f));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (Colours::edge.brighter (0.3f));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);
    juce::Path arrow;
    const float ax = (float) w - 12.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (Colours::amber);
    g.fillPath (arrow);
}

juce::Font LookAndFeel::getComboBoxFont (juce::ComboBox&)        { return juce::FontOptions (12.0f); }
juce::Font LookAndFeel::getPopupMenuFont()                       { return juce::FontOptions (13.0f); }
juce::Font LookAndFeel::getTextButtonFont (juce::TextButton&, int) { return juce::FontOptions (11.0f, juce::Font::bold); }
juce::Font LookAndFeel::getLabelFont (juce::Label& l)            { return l.getFont(); }

//==============================================================================
// KNOB / CHOICE
//==============================================================================
Knob::Knob (juce::AudioProcessorValueTreeState& s, const juce::String& paramId, const juce::String& name, int mi)
    : modIndex (mi), param (s.getParameter (paramId))
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled (false, false, nullptr);
    addAndMakeVisible (slider);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, paramId, slider);
    slider.setDoubleClickReturnValue (true, (double) param->convertFrom0to1 (param->getDefaultValue()));

    nameLabel.setText (name.toUpperCase(), juce::dontSendNotification);
    nameLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setColour (juce::Label::textColourId, Colours::dim.brighter (0.3f));
    nameLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (nameLabel);

    valueLabel.setFont (juce::FontOptions (11.0f));
    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setColour (juce::Label::textColourId, Colours::cream);
    valueLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (valueLabel);

    slider.onValueChange = [this] { refreshValue(); };
    refreshValue();
}

void Knob::refreshValue()
{
    valueLabel.setText (param->getCurrentValueAsText() + (param->getLabel().isNotEmpty() ? " " + param->getLabel() : juce::String()),
                        juce::dontSendNotification);
}

void Knob::setMod (float v)
{
    if (std::abs (v - lastMod) < 0.002f) return;
    lastMod = v;
    slider.getProperties().set ("mod", v);
    slider.repaint();
}

void Knob::resized()
{
    auto r = getLocalBounds();
    valueLabel.setBounds (r.removeFromBottom (14));
    nameLabel.setBounds (r.removeFromBottom (13));
    slider.setBounds (r.reduced (2));
}

Choice::Choice (juce::AudioProcessorValueTreeState& s, const juce::String& paramId, const juce::String& name)
{
    nameLabel.setText (name.toUpperCase(), juce::dontSendNotification);
    nameLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    nameLabel.setColour (juce::Label::textColourId, Colours::dim.brighter (0.3f));
    nameLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (nameLabel);

    if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (s.getParameter (paramId)))
        box.addItemList (c->choices, 1);
    addAndMakeVisible (box);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (s, paramId, box);
}

void Choice::resized()
{
    auto r = getLocalBounds();
    nameLabel.setBounds (r.removeFromTop (14));
    box.setBounds (r.removeFromTop (24));
}

//==============================================================================
// GRAIN CLOUD
//==============================================================================
juce::Rectangle<float> GrainCloud::laneBounds (int lane, int numLanes) const
{
    auto r = getLocalBounds().toFloat().reduced (10.0f, 8.0f);
    const float h = r.getHeight() / (float) juce::jmax (1, numLanes);
    return { r.getX(), r.getY() + h * (float) lane, r.getWidth(), h };
}

int GrainCloud::laneAt (float y) const
{
    const int n = (int) proc.getUiPool().size();
    for (int i = 0; i < n; ++i)
        if (laneBounds (i, n).contains (laneBounds (i, n).getCentreX(), y)) return i;
    return -1;
}

void GrainCloud::addEvent (const GrainEvent& e)
{
    if (particles.size() > 900) particles.erase (particles.begin(), particles.begin() + 100);
    particles.push_back ({ e.sampleIndex, e.position, juce::jlimit (-1.0f, 1.0f, e.pitch / 24.0f), e.pan,
                           0.0f, juce::jlimit (0.25f, 3.0f, e.life * 1.6f), e.reverse });
}

void GrainCloud::tick (float dt)
{
    for (auto& p : particles)
    {
        p.age += dt;
        // i grani "ballano": derivano lungo la direzione di lettura
        p.x += (p.rev ? -1.0f : 1.0f) * dt * 0.01f;
    }
    particles.erase (std::remove_if (particles.begin(), particles.end(),
                                     [] (const Particle& p) { return p.age >= p.life; }), particles.end());
}

void GrainCloud::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (Colours::panel.brighter (0.03f), area.getX(), area.getY(),
                                             Colours::bg, area.getX(), area.getBottom(), false));
    g.fillRoundedRectangle (area, 8.0f);

    const auto& pool = proc.getUiPool();
    const int n = (int) pool.size();

    if (n == 0)
    {
        g.setColour (Colours::dim);
        g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
        g.drawText (proc.isLoading() ? "loading..." : "DROP SAMPLES HERE", area.withTrimmedBottom (24.0f), juce::Justification::centred);
        g.setFont (juce::FontOptions (12.0f));
        g.drawText ("wav  aif  flac  mp3  ogg  folders  -  drag as many as you want",
                    area.withTrimmedTop (34.0f), juce::Justification::centred);
    }
    else
    {
        const int selected = juce::jlimit (0, n - 1, (int) std::round (proc.apvts.getRawParameterValue ("sample")->load() * (float) (n - 1)));

        for (int i = 0; i < n; ++i)
        {
            const auto lb = laneBounds (i, n);
            const auto& peaks = pool[(size_t) i]->peaks;
            const float cy = lb.getCentreY();
            const float hh = lb.getHeight() * 0.42f;

            if (i == selected)
            {
                g.setColour (Colours::amber.withAlpha (0.06f));
                g.fillRect (lb);
            }

            g.setColour ((i == selected ? Colours::cream : Colours::dim).withAlpha (0.35f));
            const float bw = lb.getWidth() / (float) peaks.size();
            for (size_t b = 0; b < peaks.size(); ++b)
            {
                const float ph = juce::jmax (0.5f, peaks[b] * hh);
                g.fillRect (lb.getX() + (float) b * bw, cy - ph, juce::jmax (1.0f, bw - 1.0f), ph * 2.0f);
            }

            if (lb.getHeight() > 12.0f)
            {
                g.setColour (Colours::dim);
                g.setFont (juce::FontOptions (juce::jmin (11.0f, lb.getHeight() * 0.6f)));
                g.drawText (juce::String (i + 1) + "  " + pool[(size_t) i]->name, lb.reduced (4.0f, 1.0f).toNearestInt(),
                            juce::Justification::topLeft, true);
            }
            if (i > 0)
            {
                g.setColour (Colours::edge);
                g.drawHorizontalLine ((int) lb.getY(), lb.getX(), lb.getRight());
            }
        }

        // testina di posizione
        const auto full = getLocalBounds().toFloat().reduced (10.0f, 8.0f);
        const float px = full.getX() + proc.scanPhaseDisplay.load() * full.getWidth();
        g.setColour (Colours::amber.withAlpha (0.5f));
        g.drawVerticalLine ((int) px, full.getY(), full.getBottom());

        // particelle
        for (const auto& p : particles)
        {
            if (p.lane >= n) continue;
            const auto lb = laneBounds (p.lane, n);
            const float t = p.age / p.life;
            const float alpha = (1.0f - t) * (t < 0.08f ? t / 0.08f : 1.0f);
            const float x = lb.getX() + (p.x - std::floor (p.x)) * lb.getWidth();
            const float y = lb.getCentreY() - p.y * lb.getHeight() * 0.45f;
            const float rad = 1.5f + 5.0f * (1.0f - t);
            const auto col = Colours::amber.interpolatedWith (Colours::rose, p.pan * 0.5f + 0.5f);
            g.setColour (col.withAlpha (alpha * 0.18f));
            g.fillEllipse (x - rad * 2.2f, y - rad * 2.2f, rad * 4.4f, rad * 4.4f);
            g.setColour (col.withAlpha (alpha * 0.9f));
            g.fillEllipse (x - rad * 0.5f, y - rad * 0.5f, rad, rad);
        }
    }

    g.setColour (dragOver ? Colours::amber : Colours::edge);
    g.drawRoundedRectangle (area.reduced (0.5f), 8.0f, dragOver ? 2.0f : 1.0f);
}

void GrainCloud::mouseDown (const juce::MouseEvent& e)
{
    const int n = (int) proc.getUiPool().size();
    const int lane = laneAt ((float) e.y);
    if (lane < 0) return;

    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu m;
        m.addSectionHeader (proc.getUiPool()[(size_t) lane]->name);
        m.addItem (1, "Remove sample");
        m.addItem (2, "Focus this sample");
        m.showMenuAsync (juce::PopupMenu::Options(), [this, lane, n] (int r)
        {
            if (r == 1) proc.removeSample (lane);
            if (r == 2 && n > 1)
                if (auto* prm = proc.apvts.getParameter ("sample"))
                    prm->setValueNotifyingHost ((float) lane / (float) (n - 1));
        });
    }
    else if (n > 1)
    {
        if (auto* prm = proc.apvts.getParameter ("sample"))
            prm->setValueNotifyingHost ((float) lane / (float) (n - 1));
    }
}

//==============================================================================
// EDITOR
//==============================================================================
PolvereEditor::PolvereEditor (PolvereProcessor& p)
    : AudioProcessorEditor (&p), proc (p), cloud (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&laf);
    auto& s = proc.apvts;

    auto knob = [&] (const char* id, const char* name, int mod = -1) -> juce::Component*
    {
        knobs.push_back (std::make_unique<Knob> (s, id, name, mod));
        addAndMakeVisible (*knobs.back());
        return knobs.back().get();
    };
    auto choice = [&] (const char* id, const char* name) -> juce::Component*
    {
        choices.push_back (std::make_unique<Choice> (s, id, name));
        addAndMakeVisible (*choices.back());
        return choices.back().get();
    };

    addSection ("GRAIN", { knob ("pos", "Position", PolvereProcessor::modPos), knob ("spray", "Spray", PolvereProcessor::modSpray),
                           knob ("scan", "Scan"), knob ("size", "Size", PolvereProcessor::modSize), knob ("sizeRnd", "Size Rnd"),
                           knob ("density", "Density", PolvereProcessor::modDensity), knob ("densRnd", "Chaos"), knob ("chance", "Chance") },
                { "pos", "spray", "scan", "size", "sizeRnd", "density", "densRnd", "chance" });

    addSection ("PITCH", { knob ("pitch", "Pitch"), knob ("pitchRnd", "Pitch Rnd", PolvereProcessor::modPitchRnd),
                           knob ("detune", "Detune"), knob ("reverse", "Reverse"), choice ("scale", "Scale") },
                { "pitch", "pitchRnd", "detune", "reverse", "scale" });

    addSection ("COLOUR", { knob ("pan", "Pan Spread"), knob ("sample", "Sample"), knob ("sampleRnd", "Sample Rnd"),
                            knob ("cutoff", "Cutoff", PolvereProcessor::modCutoff), knob ("cutRnd", "Cutoff Rnd"), knob ("reso", "Reso"),
                            choice ("window", "Window") },
                { "pan", "sample", "sampleRnd", "cutoff", "cutRnd", "reso", "window", "ftype" });
    // il tipo di filtro condivide la colonna con la finestra
    sections.back()->cells.push_back (choice ("ftype", "Filter"));

    addSection ("MUTATE", { knob ("mutate", "Amount"), knob ("mutRate", "Rate") }, { "mutate", "mutRate" });
    addSection ("AMP", { knob ("attack", "Attack"), knob ("decay", "Decay"), knob ("sustain", "Sustain"), knob ("release", "Release") },
                { "attack", "decay", "sustain", "release" });
    addSection ("SPACE", { knob ("dlyTime", "Delay"), knob ("dlyFb", "Feedback"), knob ("dlyMix", "Dly Mix"),
                           knob ("verbSize", "Room"), knob ("verbMix", "Verb Mix"), knob ("width", "Width"), knob ("gain", "Output") },
                { "dlyTime", "dlyFb", "dlyMix", "verbSize", "verbMix", "width" });

    for (auto* b : { &loadButton, &clearButton, &diceAllButton, &undoButton, &droneButton, &keyButton })
        addAndMakeVisible (*b);
    diceAllButton.getProperties().set ("rose", true);
    droneButton.setClickingTogglesState (true);
    keyButton.setClickingTogglesState (true);
    droneAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, "drone", droneButton);
    keyAtt   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, "keytrack", keyButton);

    loadButton.onClick    = [this] { openLoader(); };
    clearButton.onClick   = [this] { proc.clearSamples(); };
    diceAllButton.onClick = [this] { dice (allDiceable(), (float) chaosSlider.getValue()); };
    undoButton.onClick    = [this] { undo(); };

    chaosSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    chaosSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    chaosSlider.setRange (0.05, 1.0);
    chaosSlider.setValue (1.0);
    chaosSlider.setColour (juce::Slider::trackColourId, Colours::rose);
    chaosSlider.setColour (juce::Slider::backgroundColourId, Colours::track);
    chaosSlider.setColour (juce::Slider::thumbColourId, Colours::cream);
    chaosSlider.setTooltip ("Dice amount: quanto lontano salta la randomizzazione");
    addAndMakeVisible (chaosSlider);
    chaosLabel.setText ("DICE AMT", juce::dontSendNotification);
    chaosLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    chaosLabel.setColour (juce::Label::textColourId, Colours::dim.brighter (0.3f));
    addAndMakeVisible (chaosLabel);

    statusLabel.setFont (juce::FontOptions (11.0f));
    statusLabel.setColour (juce::Label::textColourId, Colours::dim.brighter (0.2f));
    statusLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (statusLabel);

    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, Colours::cream.darker (0.15f));
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, Colours::bg);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, Colours::edge);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Colours::amber);
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, Colours::amber.withAlpha (0.3f));
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, Colours::panel);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId, Colours::amber);
    keyboard.setLowestVisibleKey (36);
    keyboard.setKeyWidth (16.0f);
    addAndMakeVisible (keyboard);

    addAndMakeVisible (cloud);
    proc.addChangeListener (this);

    setSize (1040, 712);
    lastTick = juce::Time::getMillisecondCounterHiRes();
    updateStatus();
    startTimerHz (30);
}

PolvereEditor::~PolvereEditor()
{
    stopTimer();
    proc.removeChangeListener (this);
    setLookAndFeel (nullptr);
}

PolvereEditor::Section& PolvereEditor::addSection (const juce::String& title, std::initializer_list<juce::Component*> cells,
                                                   const juce::StringArray& params)
{
    auto sec = std::make_unique<Section>();
    sec->title = title;
    sec->params = params;
    sec->cells = cells;
    sec->dice = std::make_unique<juce::TextButton> ("DICE");
    sec->dice->setTooltip ("Randomizza " + title);
    auto* raw = sec.get();
    sec->dice->onClick = [this, raw] { dice (raw->params, (float) chaosSlider.getValue()); };
    addAndMakeVisible (*sec->dice);
    sections.push_back (std::move (sec));
    return *sections.back();
}

juce::StringArray PolvereEditor::allDiceable() const
{
    juce::StringArray ids;
    for (auto& s : sections)
        if (s->title != "AMP") ids.addArray (s->params);
    return ids;
}

//==============================================================================
void PolvereEditor::dice (const juce::StringArray& ids, float amount)
{
    // range normalizzati "musicali" per non finire nel silenzio o nel rumore
    static const std::map<juce::String, juce::Range<float>> ranges {
        { "pos", { 0.0f, 1.0f } },     { "spray", { 0.0f, 0.8f } },   { "scan", { 0.3f, 0.7f } },
        { "size", { 0.2f, 0.9f } },    { "sizeRnd", { 0.0f, 0.8f } }, { "density", { 0.25f, 0.85f } },
        { "densRnd", { 0.0f, 1.0f } }, { "chance", { 0.55f, 1.0f } }, { "pitch", { 0.25f, 0.75f } },
        { "pitchRnd", { 0.0f, 0.55f } }, { "detune", { 0.0f, 0.4f } }, { "reverse", { 0.0f, 0.7f } },
        { "pan", { 0.2f, 1.0f } },     { "cutoff", { 0.35f, 1.0f } }, { "cutRnd", { 0.0f, 0.6f } },
        { "reso", { 0.0f, 0.6f } },    { "mutate", { 0.05f, 0.7f } }, { "mutRate", { 0.0f, 0.65f } },
        { "attack", { 0.0f, 0.6f } },  { "decay", { 0.2f, 0.8f } },   { "sustain", { 0.3f, 1.0f } },
        { "release", { 0.25f, 0.85f } }, { "dlyFb", { 0.0f, 0.8f } }, { "dlyMix", { 0.0f, 0.5f } },
        { "verbMix", { 0.05f, 0.65f } }, { "width", { 0.4f, 0.85f } }, { "verbSize", { 0.3f, 1.0f } },
    };

    std::vector<float> snapshot;
    for (auto* prm : proc.getParameters()) snapshot.push_back (prm->getValue());
    undoStack.push_back (std::move (snapshot));
    if (undoStack.size() > 32) undoStack.erase (undoStack.begin());

    for (const auto& id : ids)
    {
        auto* prm = proc.apvts.getParameter (id);
        if (prm == nullptr) continue;

        juce::Range<float> r { 0.0f, 1.0f };
        if (auto it = ranges.find (id); it != ranges.end()) r = it->second;

        float target = r.getStart() + random.nextFloat() * r.getLength();
        if (id == "pitch" && random.nextFloat() < 0.5f) target = 0.5f; // metà delle volte resta in tonalità
        const float cur = prm->getValue();
        const float v = juce::jlimit (0.0f, 1.0f, cur + (target - cur) * amount);

        prm->beginChangeGesture();
        prm->setValueNotifyingHost (v);
        prm->endChangeGesture();
    }
}

void PolvereEditor::undo()
{
    if (undoStack.empty()) return;
    const auto snap = undoStack.back();
    undoStack.pop_back();
    const auto& params = proc.getParameters();
    for (int i = 0; i < juce::jmin ((int) snap.size(), params.size()); ++i)
    {
        params[i]->beginChangeGesture();
        params[i]->setValueNotifyingHost (snap[(size_t) i]);
        params[i]->endChangeGesture();
    }
}

void PolvereEditor::openLoader()
{
    chooser = std::make_unique<juce::FileChooser> ("Carica sample o cartelle", juce::File(),
                                                   PolvereProcessor::getSupportedWildcard());
    const auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles
                     | juce::FileBrowserComponent::canSelectDirectories | juce::FileBrowserComponent::canSelectMultipleItems;
    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
    {
        juce::StringArray paths;
        for (auto& f : fc.getResults()) paths.add (f.getFullPathName());
        if (! paths.isEmpty()) filesDropped (paths, 0, 0);
    });
}

//==============================================================================
bool PolvereEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    const auto wild = juce::StringArray::fromTokens (PolvereProcessor::getSupportedWildcard(), ";", "");
    for (auto& f : files)
    {
        const juce::File file (f);
        if (file.isDirectory()) return true;
        for (auto& w : wild)
            if (file.getFileName().matchesWildcard (w, true)) return true;
    }
    return false;
}

void PolvereEditor::fileDragEnter (const juce::StringArray&, int, int) { cloud.dragOver = true;  cloud.repaint(); }
void PolvereEditor::fileDragExit (const juce::StringArray&)            { cloud.dragOver = false; cloud.repaint(); }

void PolvereEditor::filesDropped (const juce::StringArray& files, int, int)
{
    cloud.dragOver = false;
    juce::Array<juce::File> list;
    const auto wild = PolvereProcessor::getSupportedWildcard();
    for (auto& f : files)
    {
        const juce::File file (f);
        if (file.isDirectory())
        {
            auto children = file.findChildFiles (juce::File::findFiles, true, wild);
            children.sort();
            list.addArray (children);
        }
        else
            list.add (file);
    }
    if (! list.isEmpty()) proc.loadFiles (list, false);
    cloud.repaint();
}

//==============================================================================
void PolvereEditor::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes();
    const float dt = (float) juce::jlimit (0.0, 0.2, (now - lastTick) * 0.001);
    lastTick = now;

    int s1, n1, s2, n2;
    const int ready = proc.eventFifo.getNumReady();
    proc.eventFifo.prepareToRead (ready, s1, n1, s2, n2);
    for (int i = 0; i < n1; ++i) cloud.addEvent (proc.eventBuffer[(size_t) (s1 + i)]);
    for (int i = 0; i < n2; ++i) cloud.addEvent (proc.eventBuffer[(size_t) (s2 + i)]);
    proc.eventFifo.finishedRead (n1 + n2);

    cloud.tick (dt);
    cloud.repaint();

    const bool mutating = proc.apvts.getRawParameterValue ("mutate")->load() > 0.001f
                       || std::abs (proc.apvts.getRawParameterValue ("scan")->load()) > 0.001f;
    for (auto& k : knobs)
        if (k->modIndex >= 0)
            k->setMod (mutating ? proc.modNorm[(size_t) k->modIndex].load() : -1.0f);

    updateStatus();
}

void PolvereEditor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    updateStatus();
    cloud.repaint();
}

void PolvereEditor::updateStatus()
{
    const auto n = proc.getUiPool().size();
    juce::String t;
    if (proc.isLoading()) t << "loading...   ";
    t << (int) n << (n == 1 ? " sample" : " samples") << "   |   "
      << proc.activeVoiceCount.load() << " voices   |   " << proc.activeGrainCount.load() << " grains";
    statusLabel.setText (t, juce::dontSendNotification);
}

//==============================================================================
void PolvereEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours::bg);

    // grana di fondo (statica, deterministica)
    juce::Random r (42);
    for (int i = 0; i < 900; ++i)
    {
        g.setColour (Colours::cream.withAlpha (r.nextFloat() * 0.035f));
        g.fillRect ((float) r.nextInt (getWidth()), (float) r.nextInt (getHeight()), 1.0f, 1.0f);
    }

    // titolo
    g.setColour (Colours::cream);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("P O L V E R E", 20, 12, 260, 30, juce::Justification::centredLeft);
    g.setColour (Colours::amber);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("granular dust engine  -  v1.0", 22, 40, 260, 14, juce::Justification::centredLeft);

    for (auto& s : sections)
    {
        const auto b = s->bounds.toFloat();
        g.setColour (Colours::panel);
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (Colours::edge);
        g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, 1.0f);
        g.setColour (s->title == "MUTATE" ? Colours::rose : Colours::amber);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (s->title, s->bounds.getX() + 12, s->bounds.getY() + 6, 120, 16, juce::Justification::centredLeft);
    }
}

void PolvereEditor::layoutSection (Section& s, juce::Rectangle<int> area)
{
    s.bounds = area;
    s.dice->setBounds (area.getRight() - 50, area.getY() + 5, 44, 18);

    auto inner = area.reduced (8, 0).withTrimmedTop (24).withTrimmedBottom (6);
    const int knobW = 68;

    // le Choice si impilano in un'unica colonna a destra
    std::vector<juce::Component*> ks, cs;
    for (auto* c : s.cells)
        (dynamic_cast<Choice*> (c) != nullptr ? cs : ks).push_back (c);

    if (! cs.empty())
    {
        auto col = inner.removeFromRight (100).reduced (4, 2);
        const int h = juce::jmin (44, col.getHeight() / (int) cs.size());
        for (auto* c : cs) c->setBounds (col.removeFromTop (h));
    }

    const int w = ks.empty() ? knobW : juce::jmax (52, juce::jmin (knobW + 12, inner.getWidth() / (int) ks.size()));
    for (auto* k : ks)
        k->setBounds (inner.removeFromLeft (w).withSizeKeepingCentre (juce::jmin (w, knobW + 6), juce::jmin (inner.getHeight(), 94)));
}

void PolvereEditor::resized()
{
    auto r = getLocalBounds().reduced (14);

    // header
    auto header = r.removeFromTop (48);
    header.removeFromLeft (270);
    auto right = header.removeFromRight (520).withSizeKeepingCentre (520, 28);
    for (auto* b : { &keyButton, &droneButton, &undoButton, &diceAllButton, &clearButton, &loadButton })
    {
        b->setBounds (right.removeFromRight (b == &keyButton ? 82 : b == &diceAllButton ? 86 : 66));
        right.removeFromRight (6);
    }
    auto chaos = header.withSizeKeepingCentre (header.getWidth(), 28);
    chaosLabel.setBounds (chaos.removeFromLeft (60));
    chaosSlider.setBounds (chaos.reduced (0, 4));

    r.removeFromTop (6);
    statusLabel.setBounds (r.removeFromTop (16));
    r.removeFromTop (4);
    cloud.setBounds (r.removeFromTop (200));
    r.removeFromTop (10);

    const int rowH = 126;
    const int gap = 10;
    auto byName = [this] (const juce::String& t) -> Section& {
        for (auto& s : sections) if (s->title == t) return *s;
        return *sections.front();
    };

    auto row1 = r.removeFromTop (rowH);
    layoutSection (byName ("GRAIN"), row1.removeFromLeft (8 * 72 + 16));
    row1.removeFromLeft (gap);
    layoutSection (byName ("PITCH"), row1);
    r.removeFromTop (gap);

    auto row2 = r.removeFromTop (rowH);
    layoutSection (byName ("COLOUR"), row2.removeFromLeft (6 * 72 + 16 + 100));
    row2.removeFromLeft (gap);
    layoutSection (byName ("MUTATE"), row2.removeFromLeft (2 * 72 + 16));
    row2.removeFromLeft (gap);
    layoutSection (byName ("AMP"), row2);
    r.removeFromTop (gap);

    auto row3 = r.removeFromTop (rowH);
    layoutSection (byName ("SPACE"), row3.removeFromLeft (7 * 72 + 16));
    row3.removeFromLeft (gap);
    keyboard.setBounds (row3.reduced (0, 18));
}
