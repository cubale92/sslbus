#pragma once
#include "PluginProcessor.h"
#include "EceeUI.h"

class Bus8MK2Panel : public juce::Component
{
public:
    explicit Bus8MK2Panel (Bus8MK2Processor&);
    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int W = 1000, H = 400;

private:
    ecee::ui::PresetBar presets;
    ecee::ui::Knob threshold, ratio, attack, release, makeup, hpf, drive, mixKnob;
    ecee::ui::BatSwitch compIn;
    ecee::ui::VUMeter vu;
};

class Bus8MK2Editor : public ecee::ui::ScaledEditor
{
public:
    explicit Bus8MK2Editor (Bus8MK2Processor& p) : ScaledEditor (p), panel (p) { initScaling (panel, Bus8MK2Panel::W, Bus8MK2Panel::H); }

private:
    Bus8MK2Panel panel;
};
