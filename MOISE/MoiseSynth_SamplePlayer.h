#pragma once
#include "MoiseSynth.h"

#ifndef MOISE_SYNTH_SAMPLEPLAYER_H
#define MOISE_SYNTH_SAMPLEPLAYER_H

struct MOISE_SamplePlayer_Sample {
	float *waveform; //Pointer to an array of floats representing the waveform of the sample.
	float rootFrequency; //The frequency at which the sample was recorded.
	float waveformRate; //The sample rate of the waveform.
	float positionRate; //Also the sample rate of the waveform? I can't remember why both of these variables exist.
	int loopStart; //The sample index at which the sample's loop starts.
	int loopEnd; //The sample index at which the sample's loop ends.
	//MOISE_Note minNote; //The minimum MOISE_Note value that can trigger this sample.
	//MOISE_Note maxNote; //The maximum MOISE_Note value that can trigger this sample.
};

/// <summary>
/// A sample bank represents a collection of samples that can be used depending on the note being played. For example, a piano sample bank may contain different samples for each octave of the piano keyboard.
/// This is not yet implemented.
/// </summary>
struct MOISE_SamplePlayer_SampleBank {
	MOISE_SamplePlayer_Sample sample[1];
};

class MoiseSynth_SamplePlayer : public MoiseSynth {

private:
	MOISE_SamplePlayer_Sample sampleData;

public:
	int activeSampleId = 0; //The ID of the sample to use for when sample banks are in use.

	/// <summary>
	/// Loads sample data when creating an instance of the sample player synth.
	/// </summary>
	/// <param name="data">Sample data pointer</param>
	/// <param name="waveformSampleCount">The number of samples in the provided sample data waveform</param>
	MoiseSynth_SamplePlayer(MOISE_SamplePlayer_Sample* data, int waveformSampleCount) {
		sampleData = *data;
		
		//for (int i = 0; i < waveformSampleCount; i++) {
		//	sampleData.waveform[i] = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - -1)));
		//}
	}

	/// <summary>
/// Initializes the synth with the provided sample rate and number of channels.
/// </summary>
/// <param name="setSampleRate">The sample rate of the synth</param>
/// <param name="setChannels">The number of channels of the synth</param>
	void Initialize(int setSampleRate, int setChannels) {
		sampleRate = setSampleRate;
		channels = setChannels;
		initialized = true;
	}

	/// <summary>
/// Used for testing purposes. This returns the value of the waveform at the provided sample index.
/// </summary>
/// <param name="sampleIndex">The sample index to check</param>
/// <returns></returns>
	float GetWaveformValue(int sampleIndex) override{
		return sampleData.waveform[sampleIndex];
	}

	virtual float* GetNextSample(double timeAdvance) override {
		//Get the current sample position by flooring the current precise position.
		currentSamplePosition = std::floor(currentPosition);

		//Get a sample from the waveform data at the currentSamplePosition.
		float centerSample = sampleData.waveform[currentSamplePosition];

		//Apply the sample to all channels. This is where panning will occur once implemented.
		for (int i = 0; i < channels; i++) {
			currentSample[i] = centerSample;
		}

		//Advance the current position based on the frequency being played, the rate, and the sample's root frequency.
		currentPosition += sampleData.positionRate * (frequency / sampleData.rootFrequency);
		while (currentPosition >= sampleData.loopEnd) {
			//Smoothly loop the sample back to the loop start point.
			currentPosition -= (sampleData.loopEnd - sampleData.loopStart);
		}

		//Returns the current sample.
		return currentSample;
	}

	virtual void NoteOn(int value) override {
		MoiseSynth::NoteOn(value);

		//In the future, I will select the sample to use based on the value, as there can be multiple samples in a bank.
		//Refer to old MOISE Unity project.
	}
};

#endif // !MOISE_SYNTH_SAMPLEPLAYER_H
