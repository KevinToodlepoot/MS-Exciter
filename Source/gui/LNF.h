#ifndef LNF_H
#define LNF_H

#include <JuceHeader.h>
#include "../config.h"

class MyLNF : public LookAndFeel_V4
{
    void drawRotarySlider(Graphics &g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, Slider &slider) override
    {
        auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (10);
        
        auto radius = jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        
        auto outerBounds = Rectangle<float> (bounds.getCentreX() - radius, bounds.getCentreY() - radius,
                                             2.0f * radius, 2.0f * radius);
        
        auto innerBounds = outerBounds.reduced(10.0f);
        
        auto outline = Colour(COLOR_5);
        
        ColourGradient fill (Colour(COLOR_5), outerBounds.getTopLeft(),
                             Colour(COLOR_4), outerBounds.getBottomRight(),
                             false);

        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = jmin (10.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;

        fill.multiplyOpacity(0.5f);
        g.setGradientFill(fill);
        g.fillEllipse(outerBounds);
        
        fill.multiplyOpacity(1.5f);
        g.setGradientFill(fill);
        g.fillEllipse(innerBounds);

        if (slider.isEnabled())
        {
            Path valueArc;
            valueArc.addCentredArc (outerBounds.getCentreX(),
                                    outerBounds.getCentreY(),
                                    arcRadius,
                                    arcRadius,
                                    0.0f,
                                    rotaryStartAngle,
                                    toAngle,
                                    true);

            g.setColour (outline);
            g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::butt));
        }

        auto thumbWidth = lineW;
        auto thumbRadius = innerBounds.getWidth() * 0.45 - thumbWidth * 0.5f;
        Point<float> thumbPoint (bounds.getCentreX() + thumbRadius * std::cos (toAngle - MathConstants<float>::halfPi),
                                 bounds.getCentreY() + thumbRadius * std::sin (toAngle - MathConstants<float>::halfPi));
        
        g.setColour (outline);
        g.fillEllipse (Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
    }
};

#endif // LNF_H
