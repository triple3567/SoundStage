/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

/* THINGS ZACH NEEDS TO DO:
	Take in midi input: modulation and pitch wheel for y and x respectively
	Map that to a circle (1m in radius but that's not important) and a vertical slider (-1m to 1m, again not important)
	Give Elias the degrees around the circle, and a double from -1 to 1 for elevation 
		^^ THE MAPPING MIGHT NEED TO BE DONE IN INCREMENTS BASED OFF OF AZIMUTHS AND WE MUST LIE TO THE USER IDK
	Calculate elevation angle based on the height, given that the radius is always 1
	Give Eric the elevation angle and the azimuth; he will figure out the closest ones
	
	Step 5: Profit
*/

//==============================================================================
SoundStageAudioProcessor::SoundStageAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    load_hrir_l(HRIR_L_DIR);
    load_hrir_r(HRIR_R_DIR);
}

SoundStageAudioProcessor::~SoundStageAudioProcessor()
{
}

//==============================================================================
const juce::String SoundStageAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SoundStageAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SoundStageAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SoundStageAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SoundStageAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SoundStageAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SoundStageAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SoundStageAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SoundStageAudioProcessor::getProgramName (int index)
{
return {};
}

void SoundStageAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void SoundStageAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SoundStageAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SoundStageAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void SoundStageAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.



    // make mono
    for (int sample = 0; sample < buffer.getNumSamples(); sample++) {
        float sampleLeft = buffer.getSample(0, sample);
        float sampleRight = buffer.getSample(1, sample);
        float monoSummed = sampleLeft + sampleRight;
        buffer.setSample(0, sample, monoSummed);
        buffer.setSample(1, sample, monoSummed);
    }

    float* left_hrtf = get_hrir_l(4, 8);
    float* right_hrtf = get_hrir_r(4, 8);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        if (channel == 0) {
            applyHRTF(channelData, left_hrtf, buffer.getNumSamples());
        }
        if (channel == 1) {
            applyHRTF(channelData, right_hrtf, buffer.getNumSamples());
        }

        // ..do something to the data...
    }
}

void SoundStageAudioProcessor::applyHRTF(float* channelData, float* hrtf, int numSamples) {
    
    int num_conv;
    int i, j, k;
    float temp;

    num_conv = numSamples + 200 - 1;
    
    //for each sample in block
    for (i = 0; i < num_conv; i++) {
       
        k = i;
        temp = 0.0;

        //for each sample in filter
        for (j = 0; j < 200; j++) {

            if (k >= 0 && k < numSamples) {
                temp = temp + (channelData[k] * hrtf[j]);
            }

            k--;
            output[i] = temp;


        }
    }

    for (i = 0; i < numSamples; i++) {
        channelData[i] = output[i];
    }

    std::fill_n(output, 1000, 0.0);
    

}

//==============================================================================
bool SoundStageAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SoundStageAudioProcessor::createEditor()
{
    return new SoundStageAudioProcessorEditor (*this);
}

//==============================================================================
void SoundStageAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SoundStageAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoundStageAudioProcessor();
}

int SoundStageAudioProcessor::load_hrir_l(std::string hrir_l_dir) {

    juce::File file = juce::File(hrir_l_dir);

    juce::String s = file.loadFileAsString();
    if (!file.exists()) {
        return -1;
    }

    int pos = 0;
    juce::StringArray stringArray;


    file.readLines(stringArray);
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 50; j++) {
            for (int k = 0; k < 200; k++) {
                hrir_l[i][j][k] = stringArray[(i * 50 * 200) + (j * 200) + k].getFloatValue();
            }
        }
    }

    return 0;
}

int SoundStageAudioProcessor::load_hrir_r(std::string hrir_r_dir) {
    juce::File file = juce::File(hrir_r_dir);

    juce::String s = file.loadFileAsString();
    if (!file.exists()) {
        return -1;
    }

    int pos = 0;
    juce::StringArray stringArray;


    file.readLines(stringArray);
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 50; j++) {
            for (int k = 0; k < 200; k++) {
                hrir_r[i][j][k] = stringArray[(i * 50 * 200) + (j * 200) + k].getFloatValue();
            }
        }
    }

    return 0;
}

float* SoundStageAudioProcessor::get_hrir_l(int az, int elevation) {
    float* value = hrir_l[az][elevation];
    return value;
}

float* SoundStageAudioProcessor::get_hrir_r(int az, int elevation) {
    float* value = hrir_r[az][elevation];
    return value;
}

std::vector<float> SoundStageAudioProcessor::cartesian_to_sperical(float x, float y, float z) {
    std::vector<float> return_vals;

    float r;
    float theta;
    float phi;

    r = sqrt((x * x) + (y * y) + (z * z));

    theta = atan2(y,x);

    phi = atan((sqrt((x * x) + (y * y))) / z);

    return_vals.push_back(r);
    return_vals.push_back(theta);
    return_vals.push_back(phi);

    return return_vals;
}

int SoundStageAudioProcessor::closest_elevation_index(float r, float theta, float phi) {

}