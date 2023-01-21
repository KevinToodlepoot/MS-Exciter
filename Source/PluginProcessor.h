#pragma once

#include <JuceHeader.h>

namespace audio
{
    using SIMD = juce::FloatVectorOperations;
    using APVTS = juce::AudioProcessorValueTreeState;

    template<typename Float>
    inline float msInSamples(Float ms, Float Fs) noexcept
    {
        return ms * Fs * static_cast<Float>(.001);
    }
}

#include "audio/MS.h"
#include "audio/Waveshapers.h"
#include "config.h"
#include "params.h"

using namespace audio;

//==============================================================================
/**
*/
class MSExciterAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MSExciterAudioProcessor();
    ~MSExciterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    APVTS apvts;

private:
    using Filter = dsp::IIR::Filter<float>;
    using Coefficients = Filter::CoefficientsPtr;
    using WaveShaper = dsp::WaveShaper<float>;
    
    //==============================================================================
    /* functions */
    void updateAll();
    void updateCoefficients(Coefficients &old, const Coefficients &replacements);
    void blockDC(AudioBuffer<float> &buffer);
    
    //==============================================================================
    /* processors */
    MS ms;
    
    std::array<Filter, 2> hpFilter, dcFilter;
    dsp::Oversampling<float> oversampling;
    dsp::ProcessorChain<WaveShaper, WaveShaper, WaveShaper> distChain;
    
    enum ChainPositions
    {
        Sigmoid,
        Arraya1,
        Arraya2
    };
    
    //==============================================================================
    /* parameters */
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> pDrive;
    SmoothedValue<float, ValueSmoothingTypes::Linear> pInGain, pOutGain, pMSMix, pExcMix, pMix, pWidth, pStereoize, pBias;
    
    //==============================================================================
    /* buffers */
    AudioBuffer<float> inBuffer, outBuffer, msBuffer, excBuffer, overBuffer;
    
    //==============================================================================
    /* variables */
    std::array<float, 2> dcFFState, dcFBState;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MSExciterAudioProcessor)
};
