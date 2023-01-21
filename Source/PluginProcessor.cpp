/*
  ==============================================================================
    TODO:
        - Stereoize still leans left
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MSExciterAudioProcessor::MSExciterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        apvts(*this, nullptr, "PARAMETERS", createParameterLayout()),
        oversampling(2, OS_FACTOR, dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple)
                            
#endif
{
    /* initialize parameters */
    pInGain.setCurrentAndTargetValue(GAIN_DEFAULT);
    pOutGain.setCurrentAndTargetValue(GAIN_DEFAULT);
    pMSMix.setCurrentAndTargetValue(MIX_MAX);
    pExcMix.setCurrentAndTargetValue(EXC_MIX_DEFAULT);
    pMix.setCurrentAndTargetValue(MIX_MAX);
    pWidth.setCurrentAndTargetValue(WIDTH_DEFAULT);
    pStereoize.setCurrentAndTargetValue(STEREOIZE_DEFAULT);
    pDrive.setCurrentAndTargetValue(DRIVE_DEFAULT);
    pBias.setCurrentAndTargetValue(BIAS_DEFAULT);
}

MSExciterAudioProcessor::~MSExciterAudioProcessor()
{
}

//==============================================================================
const juce::String MSExciterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MSExciterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MSExciterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MSExciterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MSExciterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MSExciterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MSExciterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MSExciterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MSExciterAudioProcessor::getProgramName (int index)
{
    return {};
}

void MSExciterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MSExciterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    /* initialize filter */
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    
    auto hpfCoeff = dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, HPF_CUTOFF, HPF_Q);
    auto dcCoeff = dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, DC_CUTOFF, DC_Q);
    
    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
    {
        hpFilter[ch].prepare(spec);
        updateCoefficients(hpFilter[ch].coefficients, hpfCoeff);
        
        dcFilter[ch].prepare(spec);
        updateCoefficients(dcFilter[ch].coefficients, dcCoeff);
    }
    
    ms.prepare(sampleRate, samplesPerBlock);
    
    oversampling.initProcessing(samplesPerBlock);
    
    dsp::ProcessSpec oversamplingSpec;
    spec.sampleRate = sampleRate * OS_FACTOR;
    spec.maximumBlockSize = samplesPerBlock * OS_FACTOR;
    spec.numChannels = 2;
    
    distChain.prepare(oversamplingSpec);
    
    auto& sigmoid = distChain.get<ChainPositions::Sigmoid>();
    sigmoid.functionToUse = [] (float x)
                            {
                                return 2.f / (1.0f + exp(-x)) - 1.f;
                            };
    
    auto& arraya1 = distChain.get<ChainPositions::Arraya1>();
    arraya1.functionToUse = [] (float x)
                            {
                                return ((3.f * x) / 2.f) * (1.0f - (pow(x, 2.0f) / 3.0f));
                            };
    
    auto& arraya2 = distChain.get<ChainPositions::Arraya2>();
    arraya2.functionToUse = arraya1.functionToUse;
    
    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
    {
        dcFFState[ch] = 0;
        dcFBState[ch] = 0;
    }
    
    inBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    outBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    msBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    excBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    overBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock * OS_FACTOR);
    
    /* reset params */
    pInGain.reset(sampleRate, SMOOTH_SEC);
    pOutGain.reset(sampleRate, SMOOTH_SEC);
    pMSMix.reset(sampleRate, SMOOTH_SEC);
    pExcMix.reset(sampleRate, SMOOTH_SEC);
    pMix.reset(sampleRate, SMOOTH_SEC);
    pWidth.reset(sampleRate, SMOOTH_SEC);
    pStereoize.reset(sampleRate, SMOOTH_SEC);
    pDrive.reset(sampleRate, SMOOTH_SEC);
    pBias.reset(sampleRate, SMOOTH_SEC);
    
    updateAll();
}

void MSExciterAudioProcessor::releaseResources()
{
    ms.reset();
    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
    {
        hpFilter[ch].reset();
        dcFilter[ch].reset();
    }
    
    oversampling.reset();
    distChain.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MSExciterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;
      
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo())
        return true;

    return false;
}
#endif

void MSExciterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int numSamps = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updateAll();

    auto bufferPtr = buffer.getArrayOfWritePointers();
    auto inPtr = inBuffer.getArrayOfWritePointers();
    auto outPtr = outBuffer.getArrayOfWritePointers();
    auto excPtr = excBuffer.getArrayOfWritePointers();
    auto msPtr = msBuffer.getArrayOfWritePointers();
    
    if (totalNumInputChannels == 1)
        SIMD::copy(bufferPtr[RIGHT], bufferPtr[LEFT], numSamps);
    
    /* apply input gain */
    for (int s = 0; s < numSamps; ++s)
    {
        float inGain = Decibels::decibelsToGain(pInGain.getNextValue());
        for(int ch = 0; ch < totalNumOutputChannels; ++ch)
        {
            bufferPtr[ch][s] *= inGain;
        }
    }
    
    /* copy buffer to input buffer */
    for(int ch = 0; ch < totalNumOutputChannels; ++ch)
        SIMD::copy(inPtr[ch], bufferPtr[ch], numSamps);
    
    /* MS */
    for (int s = 0; s < numSamps; ++s)
    {
        float leftSamp = inPtr[LEFT][s];
        float rightSamp = inPtr[RIGHT][s];
        ms.processSample(pWidth.getNextValue(), pStereoize.getNextValue(), &leftSamp, &rightSamp);
        
        msPtr[LEFT][s] = leftSamp;
        msPtr[RIGHT][s] = rightSamp;
    }
    
    // apply mix
    for (int s = 0; s < numSamps; ++s)
    {
        float msMix = pMSMix.getNextValue() / 100.0f;
        for (int ch = 0; ch < totalNumOutputChannels; ++ch)
        {
            msPtr[ch][s] *= msMix;
            msPtr[ch][s] += inPtr[ch][s] * (1.0f - msMix);
        }
    }
    
    // FILTER
    dsp::AudioBlock<float> inBlock(msBuffer);
    dsp::AudioBlock<float> outBlock(outBuffer);
    dsp::AudioBlock<float> excBlock(excBuffer);
    
    for (int ch = 0; ch < totalNumOutputChannels; ++ch)
    {
        auto inChannelBlock = inBlock.getSingleChannelBlock(ch);
        auto outChannelBlock = outBlock.getSingleChannelBlock(ch);
        
        dsp::ProcessContextNonReplacing<float> context(inChannelBlock, outChannelBlock);
        hpFilter[ch].process(context);
    }
    
    // DRIVE and BIAS
    float drive, bias;
    for (int s = 0; s < numSamps; ++s)
    {
        drive = pDrive.getNextValue();
        bias = pBias.getNextValue();
        for (int ch = 0; ch < totalNumOutputChannels; ++ch)
        {
            outPtr[ch][s] *= drive;
            outPtr[ch][s] += bias;
        }
    }
    
    // OVERSAMPLING and DISTORTION
    auto oversampledBlock = oversampling.processSamplesUp(outBlock);
    
    dsp::ProcessContextReplacing<float> oversampledContext(oversampledBlock);
    distChain.process(oversampledContext);
    
    oversampling.processSamplesDown(excBlock);
         
    // DC FILTER
    blockDC(excBuffer);
    
    // COMPENSATE GAIN
    excBuffer.applyGain(1.0f / sig(1.0f, drive));
    
    // Apply exciter mix
    for (int s = 0; s < numSamps; ++s)
    {
        float excMix = pExcMix.getNextValue() / 100.0f;
        
        for (int ch = 0; ch < totalNumOutputChannels; ++ch)
        {
            msPtr[ch][s] *= 1.0f - excMix;
            msPtr[ch][s] += excPtr[ch][s] * excMix;
        }
    }
    
    // apply overall mix
    for (int s = 0; s < numSamps; ++s)
    {
        float mix = pMix.getNextValue() / 100.0f;
        float outGain = Decibels::decibelsToGain(pOutGain.getNextValue());
        for (int ch = 0; ch < totalNumOutputChannels; ++ch)
        {
            bufferPtr[ch][s] *= 1.0f - mix;
            bufferPtr[ch][s] += msPtr[ch][s] * mix;
            
            bufferPtr[ch][s] *= outGain;
        }
    }
}

//==============================================================================
bool MSExciterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MSExciterAudioProcessor::createEditor()
{
    return new MSExciterAudioProcessorEditor (*this);
}

//==============================================================================
void MSExciterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
        apvts.state.writeToStream(mos);
}

void MSExciterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if ( tree.isValid() )
    {
        apvts.replaceState(tree);
        updateAll();
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MSExciterAudioProcessor();
}

//==============================================================================
void MSExciterAudioProcessor::updateAll()
{
    auto settings = getChainSettings(apvts);
    
    pInGain.setTargetValue(settings.inGain);
    pOutGain.setTargetValue(settings.outGain);
    pMSMix.setTargetValue(settings.msMix);
    pExcMix.setTargetValue(settings.excMix);
    pMix.setTargetValue(settings.mix);
    pWidth.setTargetValue(settings.width);
    pStereoize.setTargetValue(settings.stereoize);
    pDrive.setTargetValue(settings.drive);
    pBias.setTargetValue(settings.bias);
}

void MSExciterAudioProcessor::updateCoefficients(Coefficients &old, const Coefficients &replacements)
{
    *old = *replacements;
}

void MSExciterAudioProcessor::blockDC(AudioBuffer<float> &buffer)
{
    /*
     implementation based on the article from
     https://www.dsprelated.com/freebooks/filters/DC_Blocker.html
    */
    
    int numSamps = buffer.getNumSamples();
    auto ptr = buffer.getArrayOfWritePointers();
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        for (int s = 0; s < numSamps; ++s)
        {
            float x = ptr[ch][s];
            float y = x - dcFFState[ch] + DC_COEFF * dcFBState[ch];
            
            dcFFState[ch] = x;
            dcFBState[ch] = y;
            
            ptr[ch][s] = y;
        }
    }
}
