/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "gui/LNF.h"
#include "gui/CustomRotarySlider.h"

using namespace audio;

//==============================================================================
/**
*/
class MSExciterAudioProcessorEditor  : public juce::AudioProcessorEditor, public Timer
{
public:
    MSExciterAudioProcessorEditor (MSExciterAudioProcessor&);
    ~MSExciterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //==============================================================================
    void timerCallback() override;

private:
    MSExciterAudioProcessor& audioProcessor;
    
    //==============================================================================
    /* functions */
    void loadAssets();
    void loadBlobs();
    std::vector<Slider*> getSliders();
    
    //==============================================================================
    /* BG */
    std::array<Point<int>, NUM_BLOBS> blobPoints;
    std::array<Image, NUM_BLOBS> blobImgs;
    std::array<int, NUM_BLOBS> blobRadii;
    
    Random rand;
    Point<int> mousePos;
    
    //==============================================================================
    /* assets */
    Image bgImg;
    
    //==============================================================================
    /* components */
    CustomRotarySlider  inGainSlider, outGainSlider, msMixSlider, excMixSlider, mixSlider,
                        widthSlider, stereoizeSlider, driveSlider, biasSlider;
    
    /* attachments */
    APVTS::SliderAttachment inGainSliderAttachment, outGainSliderAttachment, msMixSliderAttachment,
                            excMixSliderAttachment, mixSliderAttachment, widthSliderAttachment,
                            stereoizeSliderAttachment, driveSliderAttachment, biasSliderAttachment;
    
    //==============================================================================
    /* LookAndFeel */
    MyLNF myLNF;
    
    class CustomFontLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        CustomFontLookAndFeel()
        {
            // without this custom Fonts won't work!!
            LookAndFeel::setDefaultLookAndFeel (this);

            // This can be used as one way of setting a default font
            setDefaultSansSerifTypeface (getCustomFont().getTypefacePtr());
            
        }

        static const juce::Font getCustomFont()
        {
            static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::SpaceGroteskSemiBold_ttf, BinaryData::SpaceGroteskSemiBold_ttfSize);
            return juce::Font (typeface);
        }

        juce::Typeface::Ptr getTypefaceForFont (const juce::Font& f) override
        {
            // This can be used to fully change/inject fonts.
            // For example: return different TTF/OTF based on weight of juce::Font (bold/italic/etc)
            return getCustomFont().getTypefacePtr();
        }
    private:
    } customLookAndFeel;
    
    const Typeface::Ptr lightTypeface = juce::Typeface::createSystemTypefaceFor (BinaryData::SpaceGroteskRegular_ttf, BinaryData::SpaceGroteskRegular_ttfSize);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MSExciterAudioProcessorEditor)
};
