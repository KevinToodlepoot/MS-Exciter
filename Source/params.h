#ifndef PARAMS_H
#define PARAMS_H

using namespace audio;

struct ChainSettings
{
    /* UTILITY */
    float inGain    { 0 };
    float outGain   { 0 };
    float msMix     { 0 };
    float excMix    { 0 };
    float mix       { 0 };
    
    /* MS */
    float width     { 0 };
    float stereoize { 0 };
    
    /* EXCITER */
    float drive     { 0 };
    float bias      { 0 };
};

inline ChainSettings getChainSettings(APVTS& apvts)
{
    ChainSettings settings;
    
    /* UTILITY */
    settings.inGain     = apvts.getRawParameterValue("Input Gain")->load();
    settings.outGain    = apvts.getRawParameterValue("Output Gain")->load();
    settings.msMix      = apvts.getRawParameterValue("M/S Mix")->load();
    settings.excMix     = apvts.getRawParameterValue("Exciter Mix")->load();
    settings.mix        = apvts.getRawParameterValue("Mix")->load();
    
    /* MS */
    settings.width      = apvts.getRawParameterValue("Width")->load();
    settings.stereoize  = apvts.getRawParameterValue("Stereoize")->load();
    
    /* EXCITER */
    settings.drive      = apvts.getRawParameterValue("Drive")->load();
    settings.bias       = apvts.getRawParameterValue("Bias")->load();
    
    return settings;
}

static APVTS::ParameterLayout createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    /* UTILITY */
    auto pInGain    = std::make_unique<AudioParameterFloat>(ParameterID ("Input Gain", 1), "Input Gain",
                                                            GAIN_MIN, GAIN_MAX, GAIN_DEFAULT);
    params.push_back(std::move(pInGain));
    
    auto pOutGain   = std::make_unique<AudioParameterFloat>(ParameterID ("Output Gain", 1), "Output Gain",
                                                            GAIN_MIN, GAIN_MAX, GAIN_DEFAULT);
    params.push_back(std::move(pOutGain));
    
    auto pMSMix     = std::make_unique<AudioParameterFloat>(ParameterID ("M/S Mix", 1), "M/S Mix",
                                                            MIX_MIN, MIX_MAX, MIX_MAX);
    params.push_back(std::move(pMSMix));
    
    auto pExcMix    = std::make_unique<AudioParameterFloat>(ParameterID ("Exciter Mix", 1), "Exciter Mix",
                                                            MIX_MIN, MIX_MAX, EXC_MIX_DEFAULT);
    params.push_back(std::move(pExcMix));
    
    auto pMix       = std::make_unique<AudioParameterFloat>(ParameterID ("Mix", 1), "Mix",
                                                            MIX_MIN, MIX_MAX, MIX_MAX);
    params.push_back(std::move(pMix));
    
    /* MS */
    auto pWidth     = std::make_unique<AudioParameterFloat>(ParameterID ("Width", 1), "Width",
                                                            WIDTH_MIN, WIDTH_MAX, WIDTH_DEFAULT);
    params.push_back(std::move(pWidth));
    
    auto pStereoize = std::make_unique<AudioParameterFloat>(ParameterID ("Stereoize", 1), "Stereoize",
                                                            STEREOIZE_MIN, STEREOIZE_MAX, STEREOIZE_DEFAULT);
    params.push_back(std::move(pStereoize));
    
    /* EXCITER */
    auto pDrive     = std::make_unique<AudioParameterFloat>(ParameterID ("Drive", 1), "Drive",
                                                            DRIVE_MIN, DRIVE_MAX, DRIVE_DEFAULT);
    params.push_back(std::move(pDrive));
    
    auto pBias      = std::make_unique<AudioParameterFloat>(ParameterID ("Bias", 1), "Bias",
                                                            BIAS_MIN, BIAS_MAX, BIAS_DEFAULT);
    params.push_back(std::move(pBias));
    
    return { params.begin(), params.end() };
}

#endif // PARAMS_H
