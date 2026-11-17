// Test offline: genera due sample, li carica, suona MIDI, controlla l'audio e salva WAV + screenshot.
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

static void writeWav (const juce::File& f, const juce::AudioBuffer<float>& b, double sr)
{
    f.deleteFile();
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> w (wav.createWriterFor (new juce::FileOutputStream (f), sr,
                                               (unsigned) b.getNumChannels(), 24, {}, 0));
    w->writeFromAudioSampleBuffer (b, 0, b.getNumSamples());
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File dir (argc > 1 ? argv[1] : juce::File::getCurrentWorkingDirectory().getFullPathName());
    const double sr = 48000.0;

    // sample di test: accordo con decadimento + rumore filtrato
    juce::AudioBuffer<float> a (1, (int) sr * 3), b (2, (int) sr * 2);
    juce::Random r (1);
    for (int i = 0; i < a.getNumSamples(); ++i)
    {
        const float t = (float) i / (float) sr;
        a.setSample (0, i, std::exp (-t) * (std::sin (6.2831f * 220.0f * t) + 0.5f * std::sin (6.2831f * 330.0f * t)
                                             + 0.3f * std::sin (6.2831f * 523.0f * t)) * 0.3f);
    }
    float lp = 0;
    for (int i = 0; i < b.getNumSamples(); ++i)
    {
        lp += 0.05f * (r.nextFloat() * 2.0f - 1.0f - lp);
        b.setSample (0, i, lp * 2.0f);
        b.setSample (1, i, lp * 1.5f);
    }
    writeWav (dir.getChildFile ("test_chord.wav"), a, sr);
    writeWav (dir.getChildFile ("test_noise.wav"), b, 44100.0);

    PolvereProcessor proc;
    proc.setPlayConfigDetails (0, 2, sr, 512);
    proc.prepareToPlay (sr, 512);
    proc.loadFiles ({ dir.getChildFile ("test_chord.wav"), dir.getChildFile ("test_noise.wav") }, false);
    for (int i = 0; i < 100 && proc.getUiPool().size() < 2; ++i)
        juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
    std::printf ("pool: %d samples\n", (int) proc.getUiPool().size());
    if (proc.getUiPool().size() != 2) return 1;

    proc.apvts.getParameter ("pitchRnd")->setValueNotifyingHost (proc.apvts.getParameter ("pitchRnd")->convertTo0to1 (12.0f));
    proc.apvts.getParameter ("scale")->setValueNotifyingHost (proc.apvts.getParameter ("scale")->convertTo0to1 (4.0f));
    proc.apvts.getParameter ("cutRnd")->setValueNotifyingHost (0.5f);
    proc.apvts.getParameter ("cutoff")->setValueNotifyingHost (0.7f);
    proc.apvts.getParameter ("mutate")->setValueNotifyingHost (0.6f);

    const int total = (int) (sr * 12.0);
    juce::AudioBuffer<float> out (2, total);
    juce::AudioBuffer<float> block (2, 512);
    int maxGrains = 0;
    for (int pos = 0; pos < total; pos += 512)
    {
        const int n = juce::jmin (512, total - pos);
        block.setSize (2, n, false, false, true);
        juce::MidiBuffer midi;
        if (pos == 0)                   { midi.addEvent (juce::MidiMessage::noteOn (1, 57, 0.9f), 10); midi.addEvent (juce::MidiMessage::noteOn (1, 64, 0.7f), 300); }
        if (pos <= (int) sr * 4 && pos + n > (int) sr * 4) { midi.addEvent (juce::MidiMessage::noteOff (1, 57), 0); midi.addEvent (juce::MidiMessage::noteOff (1, 64), 0); }
        if (pos <= (int) sr * 6 && pos + n > (int) sr * 6) proc.apvts.getParameter ("drone")->setValueNotifyingHost (1.0f);
        if (pos <= (int) sr * 10 && pos + n > (int) sr * 10) proc.apvts.getParameter ("drone")->setValueNotifyingHost (0.0f);
        proc.processBlock (block, midi);
        for (int ch = 0; ch < 2; ++ch) out.copyFrom (ch, pos, block, ch, 0, n);
        maxGrains = juce::jmax (maxGrains, proc.activeGrainCount.load());
    }

    int nans = 0;
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < total; ++i)
            if (! std::isfinite (out.getSample (ch, i))) ++nans;

    auto rms = [&] (double t0, double t1) { return out.getRMSLevel (0, (int) (t0 * sr), (int) ((t1 - t0) * sr)); };
    std::printf ("NaN/Inf: %d\npeak: %.3f\nRMS note 0.5-4s: %.4f\nRMS tail 8.5-9.5s(nota off): %.5f\nRMS drone 7-10s: %.4f\nmax grains: %d\n",
                 nans, out.getMagnitude (0, total), rms (0.5, 4.0), rms (5.5, 6.0), rms (7.0, 10.0), maxGrains);
    writeWav (dir.getChildFile ("polvere_render.wav"), out, sr);

    // screenshot GUI
    {
        std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
        ed->setVisible (true);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, 60, 0.9f), 0);
        for (int k = 0; k < 40; ++k)
        {
            block.setSize (2, 512, false, false, true);
            proc.processBlock (block, midi);
            midi.clear();
            juce::MessageManager::getInstance()->runDispatchLoopUntil (35);
        }
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f);
        juce::FileOutputStream os (dir.getChildFile ("polvere_gui.png"));
        os.setPosition (0); os.truncate();
        juce::PNGImageFormat().writeImageToStream (img, os);
    }
    return nans == 0 ? 0 : 2;
}
