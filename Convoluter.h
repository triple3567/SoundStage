/*
  ==============================================================================

    Convoluter.h
    Created: 22 Apr 2021 11:17:04am
    Author:  Eric

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

class Convoluter {
    public:
        Convoluter();
        ~Convoluter();
        void applyOutput(juce::AudioBuffer<float>& buffer);
        void readInput(juce::AudioBuffer<float>& buffer);
        void setSamplesPerBlock(int samplesPerBlock);
        float elevation;
        float azimuth;
    private:
        juce::AudioBuffer<float> inputBuffer;
        juce::AudioBuffer<float> outputBuffer;
        juce::AudioBuffer<float> convolutedSignal;
        juce::AudioBuffer<float> overflowStorage;
        int bufferSize;
        int overflowSize = 200;
        int inputPos;
        int outputPos;
        int currSamplesPerBlock;
        const juce::File DATA_DIR = 
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::globalApplicationsDirectory
            );
        float hrir_l[25][50][200];
        float hrir_r[25][50][200];

        float elevation_values[50] = { -45., -39.375, -33.75, -28.125, -22.5,
                               -16.875, -11.25 , -5.625, 0., 5.625,
                                11.25, 16.875, 22.5, 28.125, 33.75,
                                39.375, 45., 50.625, 56.25, 61.875,
                                67.5, 73.125, 78.75, 84.375, 90.,
                                95.625, 101.25, 106.875, 112.5, 118.125,
                                123.75, 129.375, 135.,140.625, 146.25,
                                151.875, 157.5, 163.125, 168.75, 174.375,
                                180., 185.625, 191.25, 196.875, 202.5,
                                208.125, 213.75, 219.375,225., 230.625 };
        float azimuth_values[25] = { -80.,-65.,-55.,-45., -40.,
                                     -35., -30., -25., -20., -15.,
                                     -10.,  -5.,   0.,   5.,  10.,
                                      15., 20.,  25.,  30.,  35.,  40.,
                                      45., 55., 65., 80 };



        void convolute();
        int load_hrir_l();
        int load_hrir_r();
        float* get_hrir_l(int az, int elevation);
        float* get_hrir_r(int az, int elevation);
        juce::AudioBuffer<float> get_hrir(int az, int elevation);
        int closest_elevation_index(float elevation);
        int closest_azimuth_index(float azimuth);
        float correctAzimuth(float azimuth);
        float correctElevation(float elevation, float azimuth);
};
