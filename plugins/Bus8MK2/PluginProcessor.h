#pragma once
#include "EceeProcessorBase.h"
#include "EceeDSP.h"

/** Bus-8 MkII — console-style VCA bus compressor with console drive, continuous sidechain HPF,
    parallel mix and factory presets voiced for 90s boom bap and new jack swing. */
class Bus8MK2Processor : public ecee::ProcessorBase
{
public:
    Bus8MK2Processor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void reset() override;
    using ProcessorBase::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    juce::AudioProcessorValueTreeState& apvts() override { return state; }

    juce::AudioProcessorValueTreeState state;
    std::atomic<float> meterGr { 0.0f };

    static juce::StringArray ratioNames()   { return { "1.5", "2", "3", "4", "5", "10", "20" }; }
    static juce::StringArray attackNames()  { return { ".1", ".3", "1", "3", "10", "30" }; }
    static juce::StringArray releaseNames() { return { ".1", ".3", ".6", "1.2", "AUTO" }; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void updateHpf (float hz);

    std::atomic<float>* pThresh;  std::atomic<float>* pRatio; std::atomic<float>* pAttack; std::atomic<float>* pRelease;
    std::atomic<float>* pMakeup;  std::atomic<float>* pHpf;   std::atomic<float>* pDrive;  std::atomic<float>* pMix;
    std::atomic<float>* pIn;

    // the whole signal path runs at 2x so the drive stage doesn't alias and dry/wet stay phase-aligned
    juce::dsp::Oversampling<float> oversampler { 2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true };
    double osRate = 88200.0;

    float det = 0.0f, grFast = 0.0f, grSlow = 0.0f, gr = 0.0f;
    ecee::Biquad scHpf[2];
    ecee::DCBlocker dc[2];
    juce::SmoothedValue<float> makeup, mix, thresh, drive, inBlend;
    float lastHpf = -1.0f;
};
