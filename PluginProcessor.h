/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <typeinfo>
#include <vector>
#include <string>
#include <math.h>
#include <cmath>

//==============================================================================
/**
*/
class SoundStageAudioProcessor  : public juce::AudioProcessor
{
public:

    //==============================================================================
    SoundStageAudioProcessor();
    ~SoundStageAudioProcessor() override;

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

    //===================================st===========================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    int load_hrir_l(std::string hrir_l_dir);
    int load_hrir_r(std::string hrir_l_dir);
    float* get_hrir_l(int az, int elevation);
    float* get_hrir_r(int az, int elevation);
    std::vector<float> cartesian_to_sperical(float x, float y, float z);


    const std::string HRIR_L_DIR = "C:/Users/Eric/Dropbox/Semester 7/3D Audio/SoundStage/Source/data/hrir_l.txt";
    const std::string HRIR_R_DIR = "C:/Users/Eric/Dropbox/Semester 7/3D Audio/SoundStage/Source/data/hrir_r.txt";
    float hrir_l[25][50][200];
    float hrir_r[25][50][200];
    float output[1000];
    std::vector<float> cartesian_to_sperical(float x, float y, float z);
    int closest_elevation_index(float r, float theta, float phi);
    int closest_azimuth_index(float r, float theta, float phi);
    

    float elevation_values[50] = { -45., -39.375, -33.75, -28.125, -22.5, -16.875, -11.25 , -5.625, 0., 5.625, 11.25, 16.875, 22.5, 28.125, 33.75, 39.375, 45., 50.625, 56.25, 61.875, 67.5, 73.125, 78.75, 84.375, 90., 95.625, 101.25, 106.875, 112.5, 118.125, 123.75, 129.375, 135.,140.625, 146.25, 151.875, 157.5, 163.125, 168.75, 174.375, 180., 185.625, 191.25, 196.875, 202.5, 208.125, 213.75, 219.375,225., 230.625 };

    float azimuth_values[25] = { -80.,-65.,-55.,-45., -40., -35., -30., -25., -20., -15., -10.,  -5.,   0.,   5.,  10.,  15.,
        20.,  25.,  30.,  35.,  40.,  45., 55., 65., 80 };

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundStageAudioProcessor)

    void applyHRTF(float* channelData, float* hrtf, int numSamples);

};
