#include "PluginEditor.h"

using namespace polvere;

//==============================================================================
// LOOK AND FEEL
//==============================================================================
LookAndFeel::LookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Colours::paper);
    setColour (juce::Label::textColourId, Colours::ink);
    setColour (juce::ComboBox::backgroundColourId, Colours::paper);
    setColour (juce::ComboBox::textColourId, Colours::ink);
    setColour (juce::ComboBox::outlineColourId, Colours::ink);
    setColour (juce::ComboBox::arrowColourId, Colours::ink);
    setColour (juce::PopupMenu::backgroundColourId, Colours::paper);
    setColour (juce::PopupMenu::textColourId, Colours::ink);
    setColour (juce::PopupMenu::headerTextColourId, Colours::inkSoft);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours::signal);
    setColour (juce::PopupMenu::highlightedTextColourId, Colours::paper);
    setColour (juce::TextButton::buttonColourId, Colours::paper);
    setColour (juce::TextButton::buttonOnColourId, Colours::ink);
    setColour (juce::TextButton::textColourOffId, Colours::ink);
    setColour (juce::TextButton::textColourOnId, Colours::paper);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                    float a0, float a1, juce::Slider&)
{
    const auto c = juce::Rectangle<int> (x, y, w, h).toFloat().getCentre();
    const float r = (float) juce::jmin (w, h) * 0.4f;
    g.setColour (Colours::ink);
    g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
    g.setColour (Colours::paper);
    g.drawLine ({ c, c.getPointOnCircumference (r * 0.8f, a0 + pos * (a1 - a0)) }, 2.0f);
}

// fader verticale da desk: fessura nera, scala serigrafata, cappuccio nero con riga bianca
void LookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float,
                                    juce::Slider::SliderStyle, juce::Slider& s)
{
    const float cx = (float) x + (float) w * 0.5f;
    const float top = (float) y, bot = (float) (y + h);

    // scala: 11 tacche, quelle a 0/50/100% più lunghe
    g.setColour (Colours::ink.withAlpha (0.75f));
    for (int i = 0; i <= 10; ++i)
    {
        const float ty = bot - (bot - top) * (float) i / 10.0f;
        const float len = (i % 5 == 0) ? 7.0f : 4.0f;
        g.fillRect (cx - 13.0f - len, ty - 0.5f, len, 1.0f);
        g.fillRect (cx + 13.0f, ty - 0.5f, len, 1.0f);
    }

    // fessura
    g.setColour (Colours::ink);
    g.fillRoundedRectangle (cx - 2.0f, top - 4.0f, 4.0f, bot - top + 8.0f, 2.0f);

    // modulazione caos: freccia rossa sul lato
    const float mod = (float) s.getProperties().getWithDefault ("mod", -1.0f);
    if (mod >= 0.0f)
    {
        const float my = bot - (bot - top) * juce::jlimit (0.0f, 1.0f, mod);
        juce::Path tri;
        tri.addTriangle (cx + 22.0f, my - 4.0f, cx + 22.0f, my + 4.0f, cx + 16.0f, my);
        g.setColour (Colours::signal);
        g.fillPath (tri);
    }

    // cappuccio
    const auto cap = juce::Rectangle<float> (cx - 11.0f, pos - 7.0f, 22.0f, 14.0f);
    g.setColour (Colours::ink.withAlpha (0.25f));
    g.fillRoundedRectangle (cap.translated (1.0f, 1.5f), 2.0f);
    g.setColour (s.isMouseOverOrDragging() ? Colours::ink.brighter (0.25f) : Colours::ink);
    g.fillRoundedRectangle (cap, 2.0f);
    g.setColour (Colours::paper);
    g.fillRect (cap.getX() + 3.0f, cap.getCentreY() - 0.75f, cap.getWidth() - 6.0f, 1.5f);
}

void LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.75f);
    const bool on = b.getToggleState();
    const bool accent = b.getProperties().getWithDefault ("rose", false);
    const auto ink = accent ? Colours::signal : Colours::ink;

    g.setColour (on || down ? ink : (over ? Colours::paperDark : Colours::paper));
    g.fillRect (r);
    g.setColour (ink);
    g.drawRect (r, 1.5f);

    // LED
    const float d = 6.0f;
    const auto led = juce::Rectangle<float> (r.getX() + 8.0f, r.getCentreY() - d * 0.5f, d, d);
    g.setColour (on ? Colours::signal.brighter (0.3f) : Colours::ink.withAlpha (0.18f));
    g.fillEllipse (led);
    if (on) { g.setColour (Colours::signal.withAlpha (0.35f)); g.fillEllipse (led.expanded (3.0f)); }
}

void LookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down)
{
    const bool accent = b.getProperties().getWithDefault ("rose", false);
    g.setFont (getTextButtonFont (b, b.getHeight()));
    g.setColour (b.getToggleState() || down ? Colours::paper : (accent ? Colours::signal : Colours::ink));
    g.drawText (b.getButtonText().toLowerCase(), b.getLocalBounds().withTrimmedLeft (12), juce::Justification::centred, false);
}

void LookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.75f);
    g.setColour (Colours::paper);
    g.fillRect (r);
    g.setColour (Colours::ink);
    g.drawRect (r, 1.2f);
    juce::Path arrow;
    const float ax = (float) w - 12.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.fillPath (arrow);
}

juce::Font LookAndFeel::getComboBoxFont (juce::ComboBox&)        { return juce::FontOptions (12.0f); }
juce::Font LookAndFeel::getPopupMenuFont()                       { return juce::FontOptions (13.0f); }
juce::Font LookAndFeel::getTextButtonFont (juce::TextButton&, int) { return juce::FontOptions (12.0f, juce::Font::bold); }
juce::Font LookAndFeel::getLabelFont (juce::Label& l)            { return l.getFont(); }

//==============================================================================
// MIDI LEARN BADGE
//==============================================================================
void polvere::drawLearnBadge (juce::Graphics& g, juce::Component& c, juce::Rectangle<int> area)
{
    const auto& pr = c.getProperties();
    const bool learning = pr.getWithDefault ("learning", false);
    const int cc = pr.getWithDefault ("cc", -1);

    if (learning)
    {
        g.setColour (Colours::signal);
        g.drawRoundedRectangle (area.toFloat().reduced (1.0f), 6.0f, 2.0f);
        g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
        g.drawText ("LEARN", area.removeFromTop (12), juce::Justification::centred);
    }
    else if (cc >= 0)
    {
        g.setColour (Colours::signal);
        g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
        g.drawText ("CC" + juce::String (cc), area.removeFromTop (11).withTrimmedRight (1), juce::Justification::topRight);
    }
}

//==============================================================================
// KNOB / CHOICE
//==============================================================================
Knob::Knob (juce::AudioProcessorValueTreeState& s, const juce::String& paramId, const juce::String& name, int mi)
    : modIndex (mi), param (s.getParameter (paramId))
{
    slider.paramId = paramId;
    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setVelocityBasedMode (false);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled (false, false, nullptr);
    addAndMakeVisible (slider);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, paramId, slider);
    slider.setDoubleClickReturnValue (true, (double) param->convertFrom0to1 (param->getDefaultValue()));

    nameLabel.setText (name.toLowerCase(), juce::dontSendNotification);
    nameLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setColour (juce::Label::textColourId, Colours::ink);
    nameLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (nameLabel);

    valueLabel.setFont (juce::FontOptions (10.0f));
    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setColour (juce::Label::textColourId, Colours::inkSoft);
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
    valueLabel.setBounds (r.removeFromBottom (13));
    nameLabel.setBounds (r.removeFromBottom (14));
    slider.setBounds (r.reduced (0, 9));
}

Choice::Choice (juce::AudioProcessorValueTreeState& s, const juce::String& paramId, const juce::String& name)
{
    box.paramId = paramId;
    nameLabel.setText (name.toLowerCase(), juce::dontSendNotification);
    nameLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    nameLabel.setColour (juce::Label::textColourId, Colours::ink);
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
// PATTERN VIEW
//==============================================================================
void PatternView::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    g.setColour (Colours::bg.withAlpha (0.6f));
    g.fillRoundedRectangle (area, 6.0f);

    const int steps = juce::jlimit (1, maxSteps, (int) proc.apvts.getRawParameterValue ("steps")->load());
    const int cur = proc.currentStep.load();
    const float w = area.getWidth() / (float) maxSteps;
    const float midY = area.getCentreY();
    const float halfH = area.getHeight() * 0.42f;
    const float range = juce::jmax (1.0f, proc.apvts.getRawParameterValue ("range")->load());

    for (int i = 0; i < maxSteps; ++i)
    {
        const auto& s = proc.pattern[(size_t) i];
        auto cell = juce::Rectangle<float> (area.getX() + (float) i * w, area.getY(), w, area.getHeight()).reduced (1.5f, 3.0f);
        const bool inRange = i < steps;

        g.setColour ((i % 4 == 0 ? Colours::edge.brighter (0.25f) : Colours::edge).withAlpha (inRange ? 1.0f : 0.3f));
        g.fillRoundedRectangle (cell, 2.0f);

        if (s.on && inRange)
        {
            const float y = midY - juce::jlimit (-1.0f, 1.0f, s.semis / range) * halfH;
            const auto col = Colours::amber.interpolatedWith (Colours::rose, juce::jlimit (0.0f, 1.0f, std::abs (s.posOffset) * 2.0f));
            g.setColour (col.withAlpha (0.35f + 0.65f * s.velocity));
            const float bh = juce::jmax (4.0f, cell.getHeight() * 0.18f);
            g.fillRoundedRectangle (cell.getX() + 1.0f, y - bh * 0.5f, cell.getWidth() - 2.0f, bh, 2.0f);
            g.drawVerticalLine ((int) cell.getCentreX(), juce::jmin (y, midY), juce::jmax (y, midY));

            for (int r = 1; r < s.ratchet; ++r) // tacche dei ratchet
                g.fillRect (cell.getX() + 2.0f + (float) r * 4.0f, cell.getBottom() - 5.0f, 2.0f, 3.0f);
        }

        if (i == cur && inRange)
        {
            g.setColour (Colours::cream.withAlpha (0.85f));
            g.drawRoundedRectangle (cell, 2.0f, 1.5f);
        }
    }

    g.setColour (Colours::dim.withAlpha (0.3f));
    g.drawHorizontalLine ((int) midY, area.getX(), area.getRight());

    if (flash > 0.0f)
    {
        g.setColour (Colours::rose.withAlpha (flash * 0.35f));
        g.fillRoundedRectangle (area, 6.0f);
    }
}

//==============================================================================
// EDITOR
//==============================================================================
PolvereEditor::PolvereEditor (PolvereProcessor& p)
    : AudioProcessorEditor (&p), proc (p), cloud (p), patternView (p)
{
    setLookAndFeel (&laf);
    auto& s = proc.apvts;
    auto learn = [this] (const juce::String& id) { showLearnMenu (id); };

    auto knob = [&] (const char* id, const char* name, int mod = -1) -> juce::Component*
    {
        knobs.push_back (std::make_unique<Knob> (s, id, name, mod));
        knobs.back()->slider.onLearn = learn;
        addAndMakeVisible (*knobs.back());
        return knobs.back().get();
    };
    auto choice = [&] (const char* id, const char* name) -> juce::Component*
    {
        choices.push_back (std::make_unique<Choice> (s, id, name));
        choices.back()->box.onLearn = learn;
        addAndMakeVisible (*choices.back());
        return choices.back().get();
    };

    addSection ("SEQUENCER", { knob ("steps", "Steps"), knob ("fill", "Fill"), knob ("range", "Range"), knob ("ratchet", "Ratchet"),
                               knob ("evolve", "Evolve"), knob ("plock", "Step Lock"), knob ("gate", "Gate"), knob ("swing", "Swing"),
                               choice ("rate", "Rate") },
                { "fill", "range", "ratchet", "evolve", "plock", "gate", "swing" });

    addSection ("GRAIN", { knob ("pos", "Position", PolvereProcessor::modPos), knob ("spray", "Spray", PolvereProcessor::modSpray),
                           knob ("scan", "Scan"), knob ("size", "Size", PolvereProcessor::modSize), knob ("sizeRnd", "Size Rnd"),
                           knob ("density", "Density", PolvereProcessor::modDensity), knob ("densRnd", "Scatter"), knob ("chance", "Chance") },
                { "pos", "spray", "scan", "size", "sizeRnd", "density", "densRnd", "chance" });

    addSection ("PITCH", { knob ("pitch", "Pitch"), knob ("pitchRnd", "Pitch Rnd", PolvereProcessor::modPitchRnd),
                           knob ("detune", "Detune"), knob ("reverse", "Reverse"), choice ("scale", "Scale") },
                { "pitch", "pitchRnd", "detune", "reverse", "scale" });

    addSection ("COLOUR", { knob ("pan", "Pan Spread"), knob ("sample", "Sample"), knob ("sampleRnd", "Sample Rnd"),
                            knob ("cutoff", "Cutoff", PolvereProcessor::modCutoff), knob ("cutRnd", "Cutoff Rnd"), knob ("reso", "Reso"),
                            choice ("window", "Window"), choice ("ftype", "Filter") },
                { "pan", "sample", "sampleRnd", "cutoff", "cutRnd", "reso", "window", "ftype" });

    addSection ("CHAOS", { knob ("mutate", "Amount"), knob ("mutRate", "Rate"), choice ("modType", "Type") },
                { "mutate", "mutRate", "modType" });

    addSection ("GLITCH", { knob ("stutter", "Stutter"), knob ("crush", "Crush"), choice ("stutLen", "Repeat") },
                { "stutter", "crush", "stutLen" });

    addSection ("AMP", { knob ("attack", "Attack"), knob ("decay", "Decay"), knob ("sustain", "Sustain"), knob ("release", "Release") },
                { "decay", "sustain", "release" });

    addSection ("SPACE", { knob ("dlyTime", "Delay"), knob ("dlyFb", "Feedback"), knob ("dlyMix", "Dly Mix"),
                           knob ("verbSize", "Room"), knob ("verbMix", "Verb Mix"), knob ("width", "Width"), knob ("gain", "Output") },
                { "dlyTime", "dlyFb", "dlyMix", "verbSize", "verbMix", "width" });

    addSection ("PERFORM", { knob ("diceAmt", "Dice Amt") }, {}, false);

    for (auto* b : { &loadButton, &clearButton }) addAndMakeVisible (*b);
    setupButton (playButton, "play");
    setupButton (generateButton, "generate");
    setupButton (diceAllButton, "dice");
    setupButton (undoButton, "undo");
    setupButton (droneButton, "drone");
    setupButton (keyButton, "keytrack");

    generateButton.getProperties().set ("rose", true);
    diceAllButton.getProperties().set ("rose", true);
    for (auto* b : { &playButton, &droneButton, &keyButton }) b->setClickingTogglesState (true);
    playAtt  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, "play", playButton);
    droneAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, "drone", droneButton);
    keyAtt   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, "keytrack", keyButton);

    loadButton.onClick     = [this] { openLoader(); };
    clearButton.onClick    = [this] { proc.clearSamples(); };
    generateButton.onClick = [this] { proc.requestGenerate(); };
    diceAllButton.onClick  = [this] { proc.randomise (proc.getDiceableIds(), s_diceAmount()); };
    undoButton.onClick     = [this] { proc.undoRandomise(); };

    statusLabel.setFont (juce::FontOptions (11.0f));
    statusLabel.setColour (juce::Label::textColourId, Colours::inkSoft);
    statusLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (statusLabel);

    addAndMakeVisible (cloud);
    addAndMakeVisible (patternView);
    proc.addChangeListener (this);

    setSize (1000, 920);
    lastTick = juce::Time::getMillisecondCounterHiRes();
    updateStatus();
    startTimerHz (30);
}

PolvereEditor::~PolvereEditor()
{
    stopTimer();
    proc.cancelMidiLearn();
    proc.removeChangeListener (this);
    setLookAndFeel (nullptr);
}

void PolvereEditor::setupButton (LearnButton& b, const juce::String& paramId)
{
    b.paramId = paramId;
    b.onLearn = [this] (const juce::String& id) { showLearnMenu (id); };
    addAndMakeVisible (b);
}

PolvereEditor::Section& PolvereEditor::addSection (const juce::String& title, std::initializer_list<juce::Component*> cells,
                                                   const juce::StringArray& params, bool withDice)
{
    auto sec = std::make_unique<Section>();
    sec->title = title;
    sec->params = params;
    sec->cells = cells;
    if (withDice)
    {
        sec->dice = std::make_unique<juce::TextButton> ("DICE");
        sec->dice->setTooltip ("Randomizza " + title);
        auto* raw = sec.get();
        sec->dice->onClick = [this, raw]
        {
            proc.randomise (raw->params, s_diceAmount());
            if (raw->title == "SEQUENCER") proc.requestGenerate();
        };
        addAndMakeVisible (*sec->dice);
    }
    sections.push_back (std::move (sec));
    return *sections.back();
}

PolvereEditor::Section& PolvereEditor::section (const juce::String& title)
{
    for (auto& s : sections) if (s->title == title) return *s;
    return *sections.front();
}

float PolvereEditor::s_diceAmount() const
{
    return proc.apvts.getRawParameterValue ("diceAmt")->load();
}

//==============================================================================
void PolvereEditor::showLearnMenu (const juce::String& paramId)
{
    auto* prm = proc.apvts.getParameter (paramId);
    if (prm == nullptr) return;

    const int cc = proc.getCCForParam (paramId);
    juce::PopupMenu m;
    m.addSectionHeader (prm->getName (40) + (cc >= 0 ? "  (CC " + juce::String (cc) + ")" : juce::String()));
    m.addItem (1, "MIDI Learn - muovi un controllo");
    m.addItem (2, "Rimuovi mappatura", cc >= 0);
    if (proc.getLearningParam().isNotEmpty()) m.addItem (3, "Annulla learn");
    m.showMenuAsync (juce::PopupMenu::Options(), [this, paramId] (int r)
    {
        if (r == 1) proc.startMidiLearn (paramId);
        if (r == 2) proc.clearMidiMapping (paramId);
        if (r == 3) proc.cancelMidiLearn();
    });
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

    // pattern: lampeggia quando ne nasce uno nuovo
    const auto pv = proc.patternVersion.load();
    if (pv != lastPatternVersion) { lastPatternVersion = pv; patternView.flash = 1.0f; }
    patternView.flash = juce::jmax (0.0f, patternView.flash - dt * 3.0f);
    patternView.repaint();

    const bool mutating = proc.apvts.getRawParameterValue ("mutate")->load() > 0.001f
                       || std::abs (proc.apvts.getRawParameterValue ("scan")->load()) > 0.001f;
    for (auto& k : knobs)
        if (k->modIndex >= 0)
            k->setMod (mutating ? proc.modNorm[(size_t) k->modIndex].load() : -1.0f);

    // stato MIDI learn sui controlli
    const auto learning = proc.getLearningParam();
    auto updateLearn = [&] (juce::Component& c, const juce::String& id)
    {
        const int cc = proc.getCCForParam (id);
        const bool isLearning = learning == id;
        auto& pr = c.getProperties();
        if ((int) pr.getWithDefault ("cc", -1) != cc || (bool) pr.getWithDefault ("learning", false) != isLearning)
        {
            pr.set ("cc", cc);
            pr.set ("learning", isLearning);
            c.repaint();
        }
    };
    for (auto& k : knobs)   updateLearn (*k, k->slider.paramId);
    for (auto& c : choices) updateLearn (*c, c->box.paramId);
    for (auto* b : { &playButton, &generateButton, &diceAllButton, &undoButton, &droneButton, &keyButton })
        updateLearn (*b, b->paramId);

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
    if (proc.getLearningParam().isNotEmpty()) t << "MIDI LEARN: muovi un controllo sul controller   |   ";
    t << (int) n << (n == 1 ? " sample" : " samples") << "   |   root "
      << juce::MidiMessage::getMidiNoteName (proc.rootNote.load(), true, true, 3) << "   |   "
      << proc.activeVoiceCount.load() << " voices   |   " << proc.activeGrainCount.load() << " grains"
      << "   |   right-click = MIDI learn";
    statusLabel.setText (t, juce::dontSendNotification);
}

//==============================================================================
void PolvereEditor::paint (juce::Graphics& g)
{
    // pannello verniciato chiaro con grana leggera
    g.fillAll (Colours::paper);
    juce::Random r (7);
    for (int i = 0; i < 2600; ++i)
    {
        g.setColour (Colours::ink.withAlpha (r.nextFloat() * 0.045f));
        g.fillRect ((float) r.nextInt (getWidth()), (float) r.nextInt (getHeight()), 1.0f, 1.0f);
    }

    // viti negli angoli
    for (auto p : { juce::Point<float> (9, 9), juce::Point<float> ((float) getWidth() - 9, 9),
                    juce::Point<float> (9, (float) getHeight() - 9), juce::Point<float> ((float) getWidth() - 9, (float) getHeight() - 9) })
    {
        g.setColour (Colours::paperDark.darker (0.15f));
        g.fillEllipse (p.x - 4.5f, p.y - 4.5f, 9.0f, 9.0f);
        g.setColour (Colours::inkSoft);
        g.drawLine (p.x - 3.0f, p.y + 1.0f, p.x + 3.0f, p.y - 1.0f, 1.2f);
    }

    // marchio
    g.setColour (Colours::ink);
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText ("polvere", 24, 12, 200, 34, juce::Justification::centredLeft);
    g.setColour (Colours::signal);
    g.fillEllipse (134.0f, 34.0f, 7.0f, 7.0f);
    g.setColour (Colours::inkSoft);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("generative granular chaos unit  /  model 1.2", 26, 44, 320, 14, juce::Justification::centredLeft);

    int n = 1;
    for (auto& s : sections)
    {
        const auto b = s->bounds.toFloat();
        // riquadro serigrafato
        g.setColour (Colours::ink);
        g.drawRect (b, 1.2f);

        // linguetta del titolo: nera con testo chiaro e numero
        const juce::String title = juce::String (n++).paddedLeft ('0', 2) + "  " + s->title.toLowerCase();
        juce::GlyphArrangement ga;
        const juce::Font f (juce::FontOptions (11.0f, juce::Font::bold));
        ga.addLineOfText (f, title, 0, 0);
        const float tw = ga.getBoundingBox (0, -1, true).getWidth() + 16.0f;
        const auto tab = juce::Rectangle<float> (b.getX(), b.getY(), tw, 18.0f);
        const bool hot = s->title == "CHAOS" || s->title == "GLITCH" || s->title == "PERFORM";
        g.setColour (hot ? Colours::signal : Colours::ink);
        g.fillRect (tab);
        g.setColour (Colours::paper);
        g.setFont (f);
        g.drawText (title, tab.withTrimmedLeft (8.0f), juce::Justification::centredLeft);

        if (s->title == "GLITCH" && proc.stutterActive.load())
        {
            g.setColour (Colours::signal);
            g.fillEllipse (b.getX() + tw + 8.0f, b.getY() + 5.0f, 8.0f, 8.0f);
        }
    }
}

void PolvereEditor::layoutSection (Section& s, juce::Rectangle<int> area, int knobW)
{
    s.bounds = area;
    if (s.dice != nullptr)
        s.dice->setBounds (area.getRight() - 50, area.getY() + 5, 44, 18);

    auto inner = area.reduced (8, 0).withTrimmedTop (24).withTrimmedBottom (6);

    std::vector<juce::Component*> ks, cs;
    for (auto* c : s.cells)
        (dynamic_cast<Choice*> (c) != nullptr ? cs : ks).push_back (c);

    if (! cs.empty())
    {
        auto col = inner.removeFromRight (96).reduced (4, 2);
        const int h = juce::jmin (44, col.getHeight() / (int) cs.size());
        for (auto* c : cs) c->setBounds (col.removeFromTop (h));
    }

    const int w = ks.empty() ? knobW : juce::jmax (50, juce::jmin (knobW + 8, inner.getWidth() / (int) ks.size()));
    for (auto* k : ks)
        k->setBounds (inner.removeFromLeft (w).withSizeKeepingCentre (juce::jmin (w, knobW + 4), juce::jmin (inner.getHeight(), 124)));
}

void PolvereEditor::resized()
{
    auto r = getLocalBounds().reduced (18, 14);
    const int gap = 10;
    const int sw = 58; // larghezza di un fader

    auto header = r.removeFromTop (48);
    auto hb = header.removeFromRight (176).withSizeKeepingCentre (176, 28);
    clearButton.setBounds (hb.removeFromRight (82));
    hb.removeFromRight (12);
    loadButton.setBounds (hb);

    statusLabel.setBounds (r.removeFromTop (16));
    r.removeFromTop (4);
    cloud.setBounds (r.removeFromTop (128));
    r.removeFromTop (gap);

    // SEQUENCER: pulsanti + pattern + fader
    {
        auto& seq = section ("SEQUENCER");
        auto area = r.removeFromTop (206);
        seq.bounds = area;
        seq.dice->setBounds (area.getRight() - 56, area.getY() + 5, 50, 18);
        auto inner = area.reduced (10, 0).withTrimmedTop (26).withTrimmedBottom (6);

        auto buttons = inner.removeFromLeft (112).reduced (0, 4);
        playButton.setBounds (buttons.removeFromTop (44));
        buttons.removeFromTop (8);
        generateButton.setBounds (buttons.removeFromTop (60));
        inner.removeFromLeft (12);

        patternView.setBounds (inner.removeFromTop (46));
        inner.removeFromTop (2);

        Section tmp;
        tmp.cells = seq.cells;
        layoutSection (tmp, inner.withTrimmedTop (-24).withTrimmedBottom (-6).expanded (8, 0), sw);
    }
    r.removeFromTop (gap);

    const int rowH = 150;
    auto row1 = r.removeFromTop (rowH);
    layoutSection (section ("GRAIN"), row1.removeFromLeft (8 * (sw + 4) + 16), sw);
    row1.removeFromLeft (gap);
    layoutSection (section ("PITCH"), row1, sw);
    {
        auto pb = section ("PITCH").bounds;
        keyButton.setBounds (pb.getRight() - 106, pb.getBottom() - 46, 96, 34);
    }
    r.removeFromTop (gap);

    auto row2 = r.removeFromTop (rowH);
    layoutSection (section ("COLOUR"), row2.removeFromLeft (6 * (sw + 4) + 16 + 96), sw);
    row2.removeFromLeft (gap);
    layoutSection (section ("CHAOS"), row2.removeFromLeft (2 * (sw + 4) + 16 + 96), sw);
    row2.removeFromLeft (gap);
    layoutSection (section ("GLITCH"), row2, sw);
    r.removeFromTop (gap);

    auto row3 = r.removeFromTop (rowH);
    layoutSection (section ("AMP"), row3.removeFromLeft (4 * (sw + 4) + 16), sw);
    row3.removeFromLeft (gap);
    layoutSection (section ("SPACE"), row3.removeFromLeft (7 * (sw + 4) + 16), sw);
    row3.removeFromLeft (gap);

    // PERFORM: dado globale, undo, drone, key track
    {
        auto& perf = section ("PERFORM");
        perf.bounds = row3;
        auto inner = row3.reduced (10, 0).withTrimmedTop (26).withTrimmedBottom (8);
        auto knobArea = inner.removeFromRight (sw);
        perf.cells.front()->setBounds (knobArea.withSizeKeepingCentre (sw, juce::jmin (124, knobArea.getHeight())));
        inner.removeFromRight (6);
        auto top = inner.removeFromTop (inner.getHeight() / 2);
        diceAllButton.setBounds (top.reduced (2, 4));
        undoButton.setBounds (inner.removeFromLeft (inner.getWidth() / 2).reduced (2, 4));
        droneButton.setBounds (inner.reduced (2, 4));
    }
}
