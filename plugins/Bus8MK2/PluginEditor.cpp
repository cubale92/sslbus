#include "PluginEditor.h"

using namespace ecee::ui;

namespace
{
const juce::Colour kText  { 0xffeee9dc };
const juce::Colour kScale { 0xffcfcabc };
const juce::Colour kRed   { 0xffc8392c };

const juce::String capRed    = "cap:ffc23b2e";
const juce::String capBlue   = "cap:ff3d6fb4";
const juce::String capGrey   = "cap:ff8e939a";
const juce::String capCream  = "cap:ffe6dfcc";
const juce::String capGreen  = "cap:ff3f8f55";
const juce::String capOrange = "cap:ffe07a2a";
}

//==============================================================================
Bus8MK2Panel::Bus8MK2Panel (Bus8MK2Processor& p)
    : presets   (p),
      threshold (p.state, "threshold", "THRESHOLD", { "-30", "-20", "-10", "0", "+10" }, capRed),
      ratio     (p.state, "ratio",     "RATIO",     Bus8MK2Processor::ratioNames(),   capBlue),
      attack    (p.state, "attack",    "ATTACK ms", Bus8MK2Processor::attackNames(),  capGrey),
      release   (p.state, "release",   "RELEASE s", Bus8MK2Processor::releaseNames(), capGrey),
      makeup    (p.state, "makeup",    "MAKE-UP",   { "0", "4", "8", "12", "16", "20" }, capCream),
      hpf       (p.state, "schpf",     "SC HPF Hz", { "OFF", "", "100", "", "500" }, capGreen),
      drive     (p.state, "drive",     "DRIVE",     { "0", "", "5", "", "10" }, capOrange),
      mixKnob   (p.state, "mix",       "MIX",       { "DRY", "", "", "", "WET" }, capCream),
      compIn    (*p.state.getParameter ("compin"), { "OUT", "IN" }, "COMP", kText),
      vu ([&p] { return p.meterGr.load(); }, VUMeter::Mode::gainReduction, "GAIN RED.")
{
    addAndMakeVisible (presets);
    for (auto* k : { &threshold, &ratio, &attack, &release, &makeup, &hpf, &drive, &mixKnob })
    {
        k->setColours (kText, kScale);
        k->setPopupParent (this);
        addAndMakeVisible (*k);
    }
    for (auto* k : { &ratio, &attack, &release })
        k->setKnobScale (0.86f);
    for (auto* k : { &hpf, &drive, &mixKnob })
        k->setKnobScale (0.88f);
    addAndMakeVisible (compIn);
    addAndMakeVisible (vu);
}

void Bus8MK2Panel::paint (juce::Graphics& g)
{
    auto panel = drawRackUnit (g, getLocalBounds().toFloat(), juce::Colour (0xff474d55), juce::Colour (0xff24282d),
                               juce::Colour (0xff6b6e72), 0.025f);

    panelText (g, "BUS-8", { panel.getX() + 22.0f, 14.0f, 100.0f, 34.0f }, kText, font (28.0f, true, 0.05f), juce::Justification::centredLeft);
    panelText (g, "MkII", { panel.getX() + 116.0f, 14.0f, 60.0f, 34.0f }, kRed.brighter (0.25f), font (20.0f, true, 0.02f), juce::Justification::centredLeft);
    panelText (g, "CONSOLE BUS COMPRESSOR", { panel.getX() + 178.0f, 14.0f, 260.0f, 34.0f }, kText.withAlpha (0.8f),
               font (12.0f, true, 0.3f), juce::Justification::centredLeft);
    panelText (g, "8cee", { panel.getRight() - 120.0f, 14.0f, 96.0f, 34.0f }, kText, font (22.0f, true), juce::Justification::centredRight);

    // red pinstripe under the header
    g.setColour (kRed);
    g.fillRect (panel.getX() + 20.0f, 56.0f, panel.getWidth() - 40.0f, 2.0f);

    // meter well
    drawRecess (g, { 52.0f, 76.0f, 262.0f, 182.0f }, 6.0f, juce::Colour (0xff181a1d));

    // lower section box
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawRoundedRectangle ({ 330.0f, 262.0f, 610.0f, 124.0f }, 6.0f, 1.2f);
    panelText (g, juce::String::fromUTF8 ("SIDECHAIN  \xc2\xb7  CONSOLE  \xc2\xb7  BLEND"), { 330.0f, 370.0f, 610.0f, 14.0f }, kText.withAlpha (0.5f), font (9.5f, true, 0.3f));

    drawLed (g, { 72.0f, 290.0f }, 5.0f, juce::Colour (0xff59ff6a), true);
    panelText (g, "POWER", { 84.0f, 282.0f, 80.0f, 16.0f }, kText.withAlpha (0.8f), font (11.0f, true, 0.2f), juce::Justification::centredLeft);
    panelText (g, juce::String::fromUTF8 ("VCA  \xc2\xb7  FEED-FORWARD  \xc2\xb7  2x OS"), { 60.0f, 316.0f, 254.0f, 14.0f }, kText.withAlpha (0.45f),
               font (9.5f, true, 0.25f), juce::Justification::centredLeft);
    panelText (g, "BOOM BAP / NEW JACK EDITION", { 60.0f, 336.0f, 254.0f, 14.0f }, kRed.brighter (0.3f).withAlpha (0.8f),
               font (9.5f, true, 0.25f), juce::Justification::centredLeft);
}

void Bus8MK2Panel::resized()
{
    presets.setBounds   (500, 15, 340, 32);
    vu.setBounds        (58, 82, 250, 170);

    threshold.setBounds (318, 70, 156, 182);
    ratio.setBounds     (478, 76, 120, 150);
    attack.setBounds    (598, 76, 120, 150);
    release.setBounds   (718, 76, 120, 150);
    makeup.setBounds    (832, 70, 140, 182);

    hpf.setBounds       (348, 266, 120, 108);
    drive.setBounds     (488, 266, 120, 108);
    mixKnob.setBounds   (628, 266, 120, 108);
    compIn.setBounds    (800, 282, 120, 84);
}
