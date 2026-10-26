#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

namespace
{
constexpr float kHpfOff = 20.0f; // bottom of the SC HPF knob = filter out
}

APVTS::ParameterLayout Bus8MK2Processor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto dB = juce::AudioParameterFloatAttributes().withLabel ("dB");
    auto pct = juce::AudioParameterFloatAttributes().withLabel ("%");

    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "threshold", 1 }, "Threshold",
                    juce::NormalisableRange<float> (-30.0f, 10.0f, 0.1f), -10.0f, dB));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "ratio", 1 }, "Ratio", ratioNames(), 3));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "attack", 1 }, "Attack (ms)", attackNames(), 5));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "release", 1 }, "Release (s)", releaseNames(), 4));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "makeup", 1 }, "Make-Up",
                    juce::NormalisableRange<float> (0.0f, 20.0f, 0.1f), 0.0f, dB));

    juce::NormalisableRange<float> hpfRange (kHpfOff, 500.0f, 0.1f);
    hpfRange.setSkewForCentre (100.0f);
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "schpf", 1 }, "Sidechain HPF", hpfRange, kHpfOff,
                    juce::AudioParameterFloatAttributes().withLabel ("Hz")
                        .withStringFromValueFunction ([] (float v, int) { return v <= kHpfOff + 0.05f ? juce::String ("OFF") : juce::String (juce::roundToInt (v)) + " Hz"; })));

    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "drive", 1 }, "Console Drive",
                    juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, pct));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "mix", 1 }, "Mix",
                    juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, pct));
    p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "compin", 1 }, "Comp In", true));

    return { p.begin(), p.end() };
}

Bus8MK2Processor::Bus8MK2Processor()
    : state (*this, nullptr, "BUS8MK2", createLayout())
{
    pThresh  = state.getRawParameterValue ("threshold");
    pRatio   = state.getRawParameterValue ("ratio");
    pAttack  = state.getRawParameterValue ("attack");
    pRelease = state.getRawParameterValue ("release");
    pMakeup  = state.getRawParameterValue ("makeup");
    pHpf     = state.getRawParameterValue ("schpf");
    pDrive   = state.getRawParameterValue ("drive");
    pMix     = state.getRawParameterValue ("mix");
    pIn      = state.getRawParameterValue ("compin");

    // ratio: 0=1.5 1=2 2=3 3=4 4=5 5=10 6=20 | attack: 0=.1 1=.3 2=1 3=3 4=10 5=30 ms | release: 0=.1 1=.3 2=.6 3=1.2 s 4=AUTO
    // thresholds assume a bus peaking around -6 dBFS
    setPresets ({
        { "Init", {} },
        { "Boom Bap Drum Bus",   { { "threshold", -14 }, { "ratio", 3 }, { "attack", 5 }, { "release", 0 }, { "makeup", 4 },  { "schpf", 90 },  { "drive", 25 } }, "90s NY Boom Bap" },
        { "Dusty Break Smash",   { { "threshold", -24 }, { "ratio", 5 }, { "attack", 2 }, { "release", 0 }, { "makeup", 10 }, { "schpf", 60 },  { "drive", 45 }, { "mix", 60 } }, "90s NY Boom Bap" },
        { "SP Grit Glue",        { { "threshold", -12 }, { "ratio", 1 }, { "attack", 4 }, { "release", 4 }, { "makeup", 3 },  { "drive", 35 } }, "90s NY Boom Bap" },
        { "NY Parallel Crush",   { { "threshold", -28 }, { "ratio", 6 }, { "attack", 0 }, { "release", 0 }, { "makeup", 14 }, { "drive", 55 }, { "mix", 35 } }, "90s NY Boom Bap" },
        { "Knock Pump",          { { "threshold", -16 }, { "ratio", 5 }, { "attack", 5 }, { "release", 0 }, { "makeup", 5 },  { "drive", 20 } }, "90s NY Boom Bap" },
        { "Loop Chop Pump",      { { "threshold", -18 }, { "ratio", 3 }, { "attack", 2 }, { "release", 0 }, { "makeup", 6 },  { "drive", 30 }, { "mix", 70 } }, "90s NY Boom Bap" },
        { "Boom Bap Mix Bus",    { { "threshold", -10 }, { "ratio", 3 }, { "attack", 5 }, { "release", 4 }, { "makeup", 3 },  { "schpf", 60 },  { "drive", 20 } }, "90s NY Boom Bap" },
        { "New Jack Drum Glue",  { { "threshold", -14 }, { "ratio", 3 }, { "attack", 4 }, { "release", 1 }, { "makeup", 3 },  { "schpf", 60 },  { "drive", 15 } }, "New Jack Swing" },
        { "Swing Beat Punch",    { { "threshold", -16 }, { "ratio", 4 }, { "attack", 5 }, { "release", 1 }, { "makeup", 5 },  { "schpf", 120 }, { "drive", 20 } }, "New Jack Swing" },
        { "Gated Snare Crack",   { { "threshold", -14 }, { "ratio", 3 }, { "attack", 3 }, { "release", 0 }, { "makeup", 4 },  { "schpf", 150 }, { "drive", 30 } }, "New Jack Swing" },
        { "Kick & Bass Lock",    { { "threshold", -12 }, { "ratio", 2 }, { "attack", 4 }, { "release", 1 }, { "makeup", 3 },  { "drive", 15 } }, "New Jack Swing" },
        { "Jack Swing Mix Bus",  { { "threshold", -8 },  { "ratio", 1 }, { "attack", 5 }, { "release", 4 }, { "makeup", 2 },  { "schpf", 90 },  { "drive", 10 } }, "New Jack Swing" },
        { "R&B Vocal Bus",       { { "threshold", -12 }, { "ratio", 1 }, { "attack", 3 }, { "release", 2 }, { "makeup", 3 },  { "schpf", 150 }, { "drive", 10 } }, "R&B" },
        { "Gentle 2-Bus Glue",   { { "threshold", -6 },  { "ratio", 0 }, { "attack", 5 }, { "release", 4 }, { "makeup", 1.5f } }, "Mix & Master" },
    });
}

//==============================================================================
void Bus8MK2Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    oversampler.initProcessing ((size_t) samplesPerBlock);
    osRate = sampleRate * 2.0;
    setLatencySamples (juce::roundToInt (oversampler.getLatencyInSamples()));
    reset();
}

void Bus8MK2Processor::reset()
{
    oversampler.reset();
    grFast = grSlow = gr = det = 0.0f;
    for (auto& d : dc) { d.prepare (osRate); d.reset(); }
    for (auto& f : scHpf) f.reset();
    lastHpf = -1.0f;
    makeup.reset (osRate, 0.03);  makeup.setCurrentAndTargetValue (ecee::dbToGain (raw (pMakeup)));
    mix.reset (osRate, 0.03);     mix.setCurrentAndTargetValue (raw (pMix) * 0.01f);
    thresh.reset (osRate, 0.05);  thresh.setCurrentAndTargetValue (raw (pThresh));
    drive.reset (osRate, 0.05);   drive.setCurrentAndTargetValue (raw (pDrive) * 0.01f);
    inBlend.reset (osRate, 0.02); inBlend.setCurrentAndTargetValue (raw (pIn) > 0.5f ? 1.0f : 0.0f);
}

void Bus8MK2Processor::updateHpf (float hz)
{
    if (std::abs (hz - lastHpf) < 0.5f) return;
    lastHpf = hz;
    for (auto& f : scHpf)
    {
        if (hz <= kHpfOff + 0.05f) f.setIdentity();
        else                        f.highPass (hz, 0.707, osRate);
    }
}

namespace
{
/** Console line-amp / transformer: asymmetric soft saturation, level-compensated so the drive
    knob changes the grit and density rather than the loudness. d = 0..1 */
inline float consoleDrive (float x, float d) noexcept
{
    if (d <= 0.0f) return x;
    const float pre  = ecee::dbToGain (d * 15.0f);
    const float bias = 0.18f * d;                       // asymmetry -> even harmonics
    const float u = x * pre;
    const float y = std::tanh (u + bias) - std::tanh (bias);
    const float sat = y / (pre * (1.0f - std::tanh (bias) * std::tanh (bias)));   // unity small-signal gain
    return x + (sat - x) * std::min (1.0f, d * 2.5f);
}
}

void Bus8MK2Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh = std::min (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < buffer.getNumChannels(); ++c)
        buffer.clear (c, 0, n);
    samplesProcessed += n;
    if (numCh == 0 || n == 0) return;

    updateHpf (raw (pHpf));
    makeup.setTargetValue (ecee::dbToGain (raw (pMakeup)));
    mix.setTargetValue (raw (pMix) * 0.01f);
    thresh.setTargetValue (raw (pThresh));
    drive.setTargetValue (raw (pDrive) * 0.01f);
    inBlend.setTargetValue (raw (pIn) > 0.5f ? 1.0f : 0.0f);

    const float ratios[]   = { 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 10.0f, 20.0f };
    const float attacks[]  = { 0.1f, 0.3f, 1.0f, 3.0f, 10.0f, 30.0f };
    const float releases[] = { 100.0f, 300.0f, 600.0f, 1200.0f, -1.0f };

    const float ratio   = ratios[juce::jlimit (0, 6, juce::roundToInt (raw (pRatio)))];
    const float atkMs   = attacks[juce::jlimit (0, 5, juce::roundToInt (raw (pAttack)))];
    const float relMs   = releases[juce::jlimit (0, 4, juce::roundToInt (raw (pRelease)))];
    const bool  autoRel = relMs < 0.0f;
    const float knee    = ratio < 2.5f ? 4.0f : 2.0f;   // low ratios ease in, high ratios grab

    // the VCA detector takes a moment to "see" a transient: a short peak-hold smoothing in front of the
    // attack stage is what lets the snare's first few ms through at 10 / 30 ms attack (the classic punch)
    const float atkC     = ecee::timeCoeff (atkMs, osRate);
    const float relC     = ecee::timeCoeff (autoRel ? 100.0f : relMs, osRate);
    const float slowAtkC = ecee::timeCoeff (400.0f, osRate);
    const float slowRelC = ecee::timeCoeff (1600.0f, osRate);
    const float detRelC  = ecee::timeCoeff (6.0f, osRate);

    juce::dsp::AudioBlock<float> block (buffer.getArrayOfWritePointers(), (size_t) numCh, (size_t) n);
    auto up = oversampler.processSamplesUp (block);
    const int m = (int) up.getNumSamples();
    float* ch[2] = { up.getChannelPointer (0), numCh > 1 ? up.getChannelPointer (1) : nullptr };

    float maxGr = 0.0f;
    for (int i = 0; i < m; ++i)
    {
        const float d = drive.getNextValue();
        float in[2], drv[2];
        float peak = 0.0f;
        for (int c = 0; c < numCh; ++c)
        {
            in[c]  = ch[c][i];
            drv[c] = consoleDrive (in[c], d);
            peak = std::max (peak, std::abs (scHpf[c].process (drv[c])));    // linked stereo, feed-forward
        }

        det = std::max (peak, detRelC * det);           // instant rise, ~6 ms decay: rides the waveform, not each cycle
        const float levelDb = ecee::gainToDb (det);
        const float t = ecee::computeGR (levelDb, thresh.getNextValue(), ratio, knee);

        // fast stage: switched attack / release
        grFast = t > grFast ? atkC * grFast + (1.0f - atkC) * t
                            : relC * grFast + (1.0f - relC) * t;

        if (autoRel)
        {
            // slow stage builds up only on sustained compression; transients release on the fast stage
            grSlow = t > grSlow ? slowAtkC * grSlow + (1.0f - slowAtkC) * t
                                : slowRelC * grSlow + (1.0f - slowRelC) * t;
            gr = std::max (grFast, std::min (grSlow, grFast + 6.0f));
        }
        else
        {
            grSlow = 0.0f;
            gr = grFast;
        }

        const float on  = inBlend.getNextValue();
        const float g   = 1.0f + (ecee::dbToGain (-gr) * makeup.getNextValue() - 1.0f) * on;
        const float mx  = mix.getNextValue();

        for (int c = 0; c < numCh; ++c)
        {
            float y = drv[c] * g;
            // VCA / output amp: a hint of 2nd harmonic that grows with the drive
            y = y + (0.004f + 0.02f * d) * y * y;
            y = dc[c].process (y);
            ch[c][i] = y * mx + in[c] * (1.0f - mx);
        }
        maxGr = std::max (maxGr, gr * on);
    }

    oversampler.processSamplesDown (block);
    meterGr.store (maxGr);
}

juce::AudioProcessorEditor* Bus8MK2Processor::createEditor() { return new Bus8MK2Editor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Bus8MK2Processor(); }
