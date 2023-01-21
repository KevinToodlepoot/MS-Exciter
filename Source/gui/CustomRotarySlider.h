#ifndef CUSTOMROTARYSLIDER_H
#define CUSTOMROTARYSLIDER_H

#include <JuceHeader.h>
#include "LNF.h"
#include "../config.h"

class CustomRotarySlider : public Slider
{
public:
    CustomRotarySlider(RangedAudioParameter &rap, const String& paramName, const String& unitSuffix) :
    Slider(Slider::SliderStyle::RotaryHorizontalVerticalDrag, Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix),
    name(paramName)
    {
        setLookAndFeel(&lnf);
    }
    
    ~CustomRotarySlider()
    {
        setLookAndFeel(nullptr);
    }
    
    void paint(Graphics &g) override
    {
        auto bounds = getLocalBounds();
        auto sliderBounds = getSliderBounds();
        
        auto range = getRange();
        
        // draw slider
        auto startAng = degreesToRadians(180.0f + 45.0f);
        auto endAng = degreesToRadians(180.0f - 45.0f) + Tau;
        
        getLookAndFeel().drawRotarySlider(g,
                                          bounds.getX(),
                                          bounds.getY(),
                                          bounds.getWidth(),
                                          bounds.getHeight(),
                                          jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                          startAng,
                                          endAng,
                                          *this);
        
        String valueString;
        
        if ( !suffix.compare("%") )
            valueString = String::formatted("%.0f", getValue());
        else
            valueString = String::formatted("%.1f", getValue());
        
        valueString += suffix;
        
        Font myFont (STD_FONT_HEIGHT);
        float stringWidth = myFont.getStringWidth(valueString);
        
        Rectangle<int> textBounds;
        
        if (myFont.getStringWidth(valueString) >= sliderBounds.reduced(21.0f).getWidth())
        {
            textBounds = Rectangle<int> (sliderBounds.getCentreX() - stringWidth / 2,
                                         sliderBounds.getY() - STD_FONT_HEIGHT,
                                         stringWidth,
                                         STD_FONT_HEIGHT);
            g.setFont(Font(lightTypeface).withHeight(STD_FONT_HEIGHT));
        }
        else
        {
            textBounds = Rectangle<int> (stringWidth, STD_FONT_HEIGHT).withCentre(bounds.getCentre());
            g.setFont(myFont);
        }
        
        
        g.setColour(Colour(COLOR_5));
        g.drawText(valueString, textBounds, Justification::centred);
    }
    
    Rectangle<float> getSliderBounds() const
    {
        auto bounds = getLocalBounds().reduced(10);
        auto radius = jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        
        return Rectangle<float> (2.0f * radius, 2.0f * radius).withCentre(bounds.getCentre().toFloat());
    }
    
private:
    MyLNF lnf;
    
    RangedAudioParameter* param;
    String suffix, name;
    
    bool isBeingDragged = false;
    const Typeface::Ptr lightTypeface = juce::Typeface::createSystemTypefaceFor (BinaryData::SpaceGroteskRegular_ttf, BinaryData::SpaceGroteskRegular_ttfSize);
};

#endif // CUSTOMROTARYSLIDER_H
