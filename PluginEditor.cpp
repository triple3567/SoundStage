/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "math.h"

//==============================================================================
SoundStageAudioProcessorEditor::SoundStageAudioProcessorEditor (SoundStageAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    // ELEVATION SLIDER SETTINGS
    elevationControl.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    elevationControl.setRange(-45.f, 90.f, 5.f);
    elevationControl.setValue(0.f);
    elevationControl.setTextBoxStyle(juce::Slider::TextBoxLeft, 0, 50, 25);
    elevationControl.addListener(this);
    addAndMakeVisible(elevationControl);


    // AZIMUTH SLIDER SETTINGS
    azimuthControl.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    azimuthControl.setRange(0.f, 360.f, 1.f);
    azimuthControl.setRotaryParameters(0.f, 4 * acos(0.0f), 0);
    azimuthControl.setTextBoxStyle(juce::Slider::TextBoxBelow, 0, 50, 25);
    azimuthControl.addListener(this);
    addAndMakeVisible(azimuthControl);


    setSize (400, 300);
}

SoundStageAudioProcessorEditor::~SoundStageAudioProcessorEditor()
{
}

//==============================================================================
void SoundStageAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void SoundStageAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    elevationControl.setBounds(3 * getWidth() / 4, getHeight()/8, 100, 3 * getHeight() / 4);
    azimuthControl.setBounds(0, 50, 200, 200);
}

void SoundStageAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &elevationControl) {
        audioProcessor.elevation = elevationControl.getValue();
        DBG(elevationControl.getValue());
    }

    if (slider == &azimuthControl) {
        audioProcessor.azimuth = azimuthControl.getValue();
        DBG(azimuthControl.getValue());
    }
}