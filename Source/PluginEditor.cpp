/*
  ==============================================================================
    TODO:
        - Add labels for knobs
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MSExciterAudioProcessorEditor::MSExciterAudioProcessorEditor (MSExciterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), mousePos(0, 0),
inGainSlider(*p.apvts.getParameter("Input Gain"), "Input Gain", "dB"),
outGainSlider(*p.apvts.getParameter("Output Gain"), "Output Gain", "dB"),
msMixSlider(*p.apvts.getParameter("M/S Mix"), "M/S Mix", "%"),
excMixSlider(*p.apvts.getParameter("Exciter Mix"), "Exciter Mix", "%"),
mixSlider(*p.apvts.getParameter("Mix"), "Mix", "%"),
widthSlider(*p.apvts.getParameter("Width"), "Width", ""),
stereoizeSlider(*p.apvts.getParameter("Stereoize"), "Stereoize", ""),
driveSlider(*p.apvts.getParameter("Drive"), "Drive", ""),
biasSlider(*p.apvts.getParameter("Bias"), "Bias", ""),
inGainSliderAttachment(p.apvts, "Input Gain", inGainSlider),
outGainSliderAttachment(p.apvts, "Output Gain", outGainSlider),
msMixSliderAttachment(p.apvts, "M/S Mix", msMixSlider),
excMixSliderAttachment(p.apvts, "Exciter Mix", excMixSlider),
mixSliderAttachment(p.apvts, "Mix", mixSlider),
widthSliderAttachment(p.apvts, "Width", widthSlider),
stereoizeSliderAttachment(p.apvts, "Stereoize", stereoizeSlider),
driveSliderAttachment(p.apvts, "Drive", driveSlider),
biasSliderAttachment(p.apvts, "Bias", biasSlider)
{
    loadAssets();
    loadBlobs();

    for (auto* slider : getSliders() )
    {
        addAndMakeVisible(slider);
    }
    
    setSize (WINDOW_WIDTH, WINDOW_HEIGHT);
    startTimerHz (FRAME_RATE);
}

MSExciterAudioProcessorEditor::~MSExciterAudioProcessorEditor()
{
    for (auto* slider : getSliders() )
        slider->setLookAndFeel(nullptr);
}

//==============================================================================
void MSExciterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // draw background
    g.drawImage(bgImg, getLocalBounds().toFloat());
    
    // draw blobs
    for (int i = 0; i < NUM_BLOBS; ++i)
    {
        int x, y;
        x = blobPoints[i].getX() + ( (mousePos.getX() - WINDOW_WIDTH / 2) / (blobRadii[i]) );
        y = blobPoints[i].getY() + ( (mousePos.getY() - WINDOW_HEIGHT / 2) / (blobRadii[i]) );
        
        g.drawImage(blobImgs[i], x - blobRadii[i], y - blobRadii[i], 2 * blobRadii[i], 2 * blobRadii[i], 0, 0, blobImgs[i].getWidth(), blobImgs[i].getHeight());
    }
    
    // draw center
    auto centerColor = Colour(COLOR_1).withAlpha(0.25f);
    auto textColor = Colour(COLOR_5);
    
    const auto container = getLocalBounds().reduced(10);
    auto centerBounds = Rectangle<int>(80, container.getHeight()).withCentre(container.getCentre());
    auto excBounds = container;
    auto msBounds = excBounds.removeFromLeft(excBounds.proportionOfWidth(0.5f));
    msBounds.removeFromRight(70);
    excBounds.removeFromLeft(70);
    
    g.setColour(centerColor);
    g.fillRoundedRectangle(centerBounds.toFloat(), 5.0f);
    g.fillRoundedRectangle(msBounds.toFloat(), 5.0f);
    g.fillRoundedRectangle(excBounds.toFloat(), 5.0f);
//    g.setColour(Colour(COLOR_4));
//    g.drawRoundedRectangle(centerBounds.toFloat(), 5.0f, 1.0f);
//    g.drawRoundedRectangle(msBounds.toFloat(), 5.0f, 1.0f);
//    g.drawRoundedRectangle(excBounds.toFloat(), 5.0f, 1.0f);
    
    // draw Title
    g.setColour(textColor);
    g.setFont(60.0f);
    g.drawText("m/s", msBounds, Justification::centredTop);
    g.drawText("exciter", excBounds, Justification::centredTop);
    
    // draw Labels
    msBounds.removeFromTop(60);
    msBounds.removeFromBottom(20);
    excBounds.removeFromTop(60);
    excBounds.removeFromBottom(20);
    auto widthLabelBounds = msBounds.removeFromTop(msBounds.proportionOfHeight(0.6f));
    auto msMixLabelBounds = msBounds.removeFromLeft(msBounds.proportionOfWidth(0.5f));
    auto inGainLabelBounds = centerBounds.removeFromTop(centerBounds.proportionOfHeight(0.33f));
    auto outGainLabelBounds = centerBounds.removeFromTop(centerBounds.proportionOfHeight(0.5f));
    auto driveLabelBounds = excBounds.removeFromTop(excBounds.proportionOfHeight(0.6f));
    auto biasLabelBounds = excBounds.removeFromLeft(excBounds.proportionOfWidth(0.5f));
    
    g.setFont(Font(lightTypeface).withHeight(STD_FONT_HEIGHT));
//    g.setFont(STD_FONT_HEIGHT);
    g.drawText("width", widthLabelBounds.translated(0, 15), Justification::centredBottom);
    g.drawText("mix", msMixLabelBounds.translated(0, 15), Justification::centredBottom);
    g.drawText("stereoize", msBounds.translated(0, 15), Justification::centredBottom);
    g.drawText("in gain",
               inGainLabelBounds.withSizeKeepingCentre(inGainLabelBounds.getWidth(), inGainLabelBounds.getWidth()).translated(0, 15),
               Justification::centredBottom);
    g.drawText("out gain",
               outGainLabelBounds.withSizeKeepingCentre(outGainLabelBounds.getWidth(), outGainLabelBounds.getWidth()).translated(0, 15),
               Justification::centredBottom);
    g.drawText("mix",
               centerBounds.withSizeKeepingCentre(centerBounds.getWidth(), centerBounds.getWidth()).translated(0, 15),
               Justification::centredBottom);
    g.drawText("drive", driveLabelBounds.translated(0, 15), Justification::centredBottom);
    g.drawText("bias", biasLabelBounds.translated(0, 15), Justification::centredBottom);
    g.drawText("mix", excBounds.translated(0, 15), Justification::centredBottom);
}

void MSExciterAudioProcessorEditor::resized()
{
    const auto container = getLocalBounds().reduced(10);
    auto excBounds = container;
    auto msBounds = excBounds.removeFromLeft(excBounds.proportionOfWidth(0.5f));
    msBounds.removeFromRight(70);
    msBounds.removeFromTop(60);
    msBounds.removeFromBottom(20);
    excBounds.removeFromLeft(70);
    excBounds.removeFromTop(60);
    excBounds.removeFromBottom(20);
    auto centerBounds = Rectangle<int>(80, container.getHeight()).withCentre(container.getCentre());
    
    /* MS Section */
    widthSlider.setBounds(msBounds.removeFromTop(msBounds.proportionOfHeight(0.6f)));
    
    msMixSlider.setBounds(msBounds.removeFromLeft(msBounds.proportionOfWidth(0.5f)));
    stereoizeSlider.setBounds(msBounds);
    
    /* Center Section */
    inGainSlider.setBounds(centerBounds.removeFromTop(centerBounds.proportionOfHeight(0.33f)));
    outGainSlider.setBounds(centerBounds.removeFromTop(centerBounds.proportionOfHeight(0.5f)));
    mixSlider.setBounds(centerBounds);
    
    /* Exciter Section */
    driveSlider.setBounds(excBounds.removeFromTop(excBounds.proportionOfHeight(0.6f)));
    
    biasSlider.setBounds(excBounds.removeFromLeft(excBounds.proportionOfWidth(0.5f)));
    excMixSlider.setBounds(excBounds);
}

//==============================================================================
void MSExciterAudioProcessorEditor::timerCallback()
{
    mousePos = getMouseXYRelative();
    
    repaint();
}

//==============================================================================
void MSExciterAudioProcessorEditor::loadAssets()
{
    bgImg = ImageCache::getFromMemory(BinaryData::bg_png, BinaryData::bg_pngSize);
}

void MSExciterAudioProcessorEditor::loadBlobs()
{
    for (int i = 0; i < NUM_BLOBS; ++i)
    {
        if (i % 2)
            blobImgs[i] = ImageCache::getFromMemory(BinaryData::blueblob_png, BinaryData::blueblob_pngSize);
        else
            blobImgs[i] = ImageCache::getFromMemory(BinaryData::beigeblob_png, BinaryData::beigeblob_pngSize);
        
        blobPoints[i].setXY(rand.nextInt(WINDOW_WIDTH), rand.nextInt(WINDOW_HEIGHT));
        
        blobRadii[i] = rand.nextInt(Range<int>(BLOB_SIZE_MIN, BLOB_SIZE_MAX));
    }
}

std::vector<Slider*> MSExciterAudioProcessorEditor::getSliders()
{
    return
    {
        &inGainSlider,
        &outGainSlider,
        &msMixSlider,
        &excMixSlider,
        &mixSlider,
        &widthSlider,
        &stereoizeSlider,
        &driveSlider,
        &biasSlider
    };
}
