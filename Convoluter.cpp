/*
  ==============================================================================

    Convoluter.cpp
    Created: 22 Apr 2021 11:17:04am
    Author:  Eric

  ==============================================================================
*/

#include "Convoluter.h"

Convoluter::Convoluter() {
	currSamplesPerBlock = -1;
	bufferSize = -1;
	inputBuffer = juce::AudioBuffer<float>();
	outputBuffer = juce::AudioBuffer<float>();
	convolutedSignal = juce::AudioBuffer<float>();;
	overflowStorage = juce::AudioBuffer<float>(2, 200);

	inputPos = 0;
	outputPos = 0;
	elevation = 0;
	azimuth = 0;

	load_hrir_l();
	load_hrir_r();
}

Convoluter::~Convoluter() {

}

void Convoluter::setSamplesPerBlock(int samplesPerBlock) {

	if (samplesPerBlock != currSamplesPerBlock) {
		bufferSize = samplesPerBlock * 10;
		inputBuffer = juce::AudioBuffer<float>(2, bufferSize);
		outputBuffer = juce::AudioBuffer<float>(2, bufferSize);
		convolutedSignal = juce::AudioBuffer<float>(2, bufferSize + 200);;
		overflowStorage = juce::AudioBuffer<float>(2, 200);
	}
}

void Convoluter::readInput(juce::AudioBuffer<float>& buffer) {
	auto numInputChannels = buffer.getNumChannels();

	//for each channel add the current block to the input buffer
	for (int channel = 0; channel < numInputChannels; channel++) {
		auto* writePointer = inputBuffer.getWritePointer(channel, inputPos);
		auto* readPointer = buffer.getReadPointer(channel, 0);

		for (int i = 0; i < buffer.getNumSamples(); i++) {
			writePointer[i] = readPointer[i];
		}

	}

	inputPos += buffer.getNumSamples();

	//if the input buffer is full, convolute the signal and reset the input buffer
	if (inputPos + 1 >= bufferSize) {
		convolute();
		inputPos = 0;
		inputBuffer.clear();
	}

}

void Convoluter::convolute() {

	int num_conv = bufferSize + overflowSize ;
	int i, j, k;
	float temp;

	/*
	* for each channel, convolute the signal based on the current azimuth and elevation
	* and add the samples from the overflow storage to the begining of the covoluted signal
	*/
	for (int channel = 0; channel < convolutedSignal.getNumChannels(); channel++) {
		auto* writePointer = convolutedSignal.getWritePointer(channel);
		auto* readPointer = inputBuffer.getReadPointer(channel);
		float* hrtf;

		if (channel == 0) {
			hrtf =
				get_hrir_l(
					closest_azimuth_index(correctAzimuth(azimuth)),
					closest_elevation_index(correctElevation(elevation, azimuth))
				);
		}
		else if (channel == 1) {
			hrtf =
				get_hrir_r(
					closest_azimuth_index(correctAzimuth(azimuth)),
					closest_elevation_index(correctElevation(elevation, azimuth))
				);
		}

		for (i = 0; i < num_conv; i++) {
			k = i;
			temp = 0.0;

			//for each sample in filter
			for (j = 0; j < 200; j++) {

				if (k >= 0 && k < bufferSize) {
					temp = temp + (readPointer[k] * hrtf[j]);
				}

				k--;
				writePointer[i] = temp;


			}
		}

		//add the overflow storage to the begining of the convoluted signal
		auto* overflowPointer = overflowStorage.getReadPointer(channel);
		for (int z = 0; z < 200; z++) {
			writePointer[z] += overflowPointer[z];
		}
	}

	overflowStorage.clear();

	/*
	* push convoluted signal to the output buffer
	* generate the overflow storage from the convoluted signal
	*/ 
	for (int channel = 0; channel < convolutedSignal.getNumChannels(); channel++) {
		auto* readPointer = convolutedSignal.getReadPointer(channel);
		auto* readPointerOverflow = convolutedSignal.getReadPointer(channel, bufferSize + 1);

		outputBuffer.copyFrom(channel, 0, readPointer, bufferSize);
		overflowStorage.copyFrom(channel, 0, readPointerOverflow, overflowSize);
	}

	convolutedSignal.clear();
}

void Convoluter::applyOutput(juce::AudioBuffer<float>& buffer) {
	auto numInputChannels = buffer.getNumChannels();


	for (int channel = 0; channel < numInputChannels; channel++) {
		auto* writePointer = buffer.getWritePointer(channel);
		auto* readPointer = outputBuffer.getReadPointer(channel, outputPos);

		for (int i = 0; i < buffer.getNumSamples(); i++) {
			writePointer[i] = readPointer[i];
			//writePointer[i] = 1.0f / (float)i;
		}

	}


	outputPos += buffer.getNumSamples();

	//if the output buffer at the end, reset the output buffer
	if (outputPos + 1 >= bufferSize) {
		outputPos = 0;
		outputBuffer.clear();
	}

}

float Convoluter::correctAzimuth(float azimuth) {
	//this function takes in an angle from 0-360 and spits out the azimuth translated to what our HRTF can use
	/*Structure:*/
	if (azimuth >= 270.0 && azimuth <= 360.0) {
		return -1.0 * (360.0 - azimuth);
	}
	else if (azimuth >= 0.0 && azimuth <= 90.0) {
		return azimuth;
	}
	else if (azimuth > 90.0 && azimuth <= 180.0) {
		return (180 - azimuth);
	}
	else if (azimuth > 180.0 && azimuth < 270.0) {
		return -1.0 * (azimuth - 180.0);
	}
	else
		return 0.0;
}

float Convoluter::correctElevation(float elevation, float azimuth) {
	//this function takes in an angle from -45 to 90 and spits out the correct elevation based on the azimuth
	float elevbuff = 0.0f;
	if (azimuth >= 270.0 && azimuth <= 360.0) {
		return elevation;
	}
	else if (azimuth >= 0.0 && azimuth <= 90.0) {
		return elevation;
	}
	else if (azimuth > 90.0 && azimuth < 270.0) {
		elevbuff = elevation + (180 - 2 * elevation);
		if (elevbuff > 230.625)
			elevbuff = 230.625;
		return elevbuff;
	}
	else
		return 0.0;
}

int Convoluter::closest_elevation_index(float elevation) {
	float min_diff = std::numeric_limits<float>::max();
	int index_found = -1;

	for (int i = 0; i < 50; i++) {
		float temp = abs(elevation - elevation_values[i]);

		if (temp < min_diff) {
			min_diff = temp;
			index_found = i;
		}
	}

	return index_found;
}

int Convoluter::closest_azimuth_index(float azimuth) {
	float min_diff = std::numeric_limits<float>::max();
	int index_found = -1;

	for (int i = 0; i < 25; i++) {
		float temp = abs(azimuth - azimuth_values[i]);

		if (temp < min_diff) {
			min_diff = temp;
			index_found = i;
		}
	}

	return index_found;
}

int Convoluter::load_hrir_l() {

	juce::File file = DATA_DIR.getChildFile("SoundStage/hrir_l.txt");

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

int Convoluter::load_hrir_r() {
	juce::File file = DATA_DIR.getChildFile("SoundStage/hrir_r.txt");

	juce::String s = file.loadFileAsString();
	if (!file.exists()) {
		juce::Logger::outputDebugString("failed to load right");
		juce::Logger::outputDebugString(juce::String(file.getFullPathName()));
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

float* Convoluter::get_hrir_l(int az, int elevation) {
	float* value = hrir_l[az][elevation];

	return value;
}

float* Convoluter::get_hrir_r(int az, int elevation) {
	float* value = hrir_r[az][elevation];


	return value;
}