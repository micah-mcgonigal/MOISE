#pragma once

#include "Moise_data.h"
#include "MoiseSynth.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"

#ifndef MOISE_SYNTH_SAMPLEPLAYER_H
#define MOISE_SYNTH_SAMPLEPLAYER_H

using json = nlohmann::json;

/// <summary>
/// A sample struct contains all the data necessary for playback of a sample.
/// </summary>
struct MOISE_SamplePlayer_Sample {
	std::vector<float> waveform; //Buffer of floats representing the waveform of the sample.
	float rootFrequency; //The frequency at which the sample was recorded.
	float waveformRate; //The base sample rate of the waveform.
	int loopStart; //The sample index at which the sample's loop starts.
	int loopEnd; //The sample index at which the sample's loop ends.
	float minFrequency; //The minimum frequency that a sample should be used.
	float maxFrequency; //The maximum frequency that a sample should be used.
	//MOISE_Note minNote; //The minimum MOISE_Note value that can trigger this sample.
	//MOISE_Note maxNote; //The maximum MOISE_Note value that can trigger this sample.
	int waveformLength; //The length of the waveform (in samples).
	int waveformChunkStart; //The index (in bytes) that this sample's waveform data starts at, within the sample bank file.
	std::string name; //The name of this sample.
};

//This allows us to deserialize JSON directly into our MOISE_SamplePlayer_Sample struct
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	MOISE_SamplePlayer_Sample,
	rootFrequency,
	waveformRate,
	loopStart,
	loopEnd,
	minFrequency,
	maxFrequency,
	waveformLength,
	waveformChunkStart,
	name)

/// <summary>
/// A sample bank represents a collection of samples that can be used depending on the note being played. For example, a piano sample bank may contain different samples for each octave of the piano keyboard.
/// This is not yet implemented.
/// </summary>
struct MOISE_SamplePlayer_SampleBank {
	MOISE_SamplePlayer_Sample sample[1];
};

/// <summary>
/// A sample player is a MOISE synthesizer that implements sample playback.
/// </summary>
class MoiseSynth_SamplePlayer : public MoiseSynth {

private:
	std::vector<MOISE_SamplePlayer_Sample> sampleBank = {}; //Sample bank for this sample player.

public:
	int activeSampleId = 0; //The ID of the sample to use for when sample banks are in use. Currently this corresponds to an index into the sample bank.
	bool startedLoopingSample = false; //True once a playing sample has started looping

	/// <summary>
	/// Default constructor for a sample player.
	/// </summary>
	MoiseSynth_SamplePlayer() {}

	/// <summary>
	/// Loads sample data from memory when creating an instance of the sample player synth.
	/// </summary>
	/// <param name="data">Sample data pointer</param>
	/// <param name="waveformSampleCount">The number of samples in the provided sample data waveform</param>
	MoiseSynth_SamplePlayer(MOISE_SamplePlayer_Sample* data, int waveformSampleCount) {
		sampleBank.push_back(*data);
		
		//for (int i = 0; i < waveformSampleCount; i++) {
		//	sampleData.waveform[i] = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - -1)));
		//}
	}

	/// <summary>
	/// Load a sample into this sample player's sample bank.
	/// </summary>
	/// <param name="inSampleData">The sample data that defines the sample.</param>
	/// <param name="inSampleBankFileStream">The input file stream that holds the sample bank's waveform data.</param>
	void LoadSampleIntoBank(const MOISE_SamplePlayer_Sample& inSampleData, std::ifstream& inSampleBankFileStream) {
		// --------
		// Copy sample data

		MOISE_SamplePlayer_Sample sampleData = inSampleData;

		// --------
		// Get waveform from sample bank

		// Seek to waveform chunk
		inSampleBankFileStream.seekg(sampleData.waveformChunkStart);

		// Resize waveform buffer
		sampleData.waveform.resize(sampleData.waveformLength);

		// Load waveform file's data into our waveform buffer
		if (!inSampleBankFileStream.read(reinterpret_cast<char*>(sampleData.waveform.data()), sampleData.waveformLength * sizeof(float))) {
			std::cerr << "MoiseSynth_SamplePlayer: Failed to load sample waveform data at waveformChunkStart " << sampleData.waveformChunkStart << std::endl;
		}

		// Add sample to bank
		sampleBank.push_back(sampleData);
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
	float GetWaveformValue(int sampleIndex) override {
		if (sampleBank.size() < 1) {
			return 0.f;
		}

		//Get active sample data
		const MOISE_SamplePlayer_Sample& sampleData = sampleBank[activeSampleId];

		if (sampleData.waveform.size() <= sampleIndex) {
			return 0.f;
		}

		//Get value at the given index in the sample data's waveform
		return sampleData.waveform[sampleIndex];
	}

	/// <summary>
	/// Resampling interpolation. Source: https://stackoverflow.com/questions/1125666/how-do-you-do-bicubic-or-other-non-linear-interpolation-of-re-sampled-audio-da
	/// More info: https://stackoverflow.com/questions/5390957/explanation-of-interpolate-hermite-method
	/// </summary>
	/// <param name="x0">Sample N - 1</param>
	/// <param name="x1">Sample N</param>
	/// <param name="x2">Sample N + 1</param>
	/// <param name="x3">Sample N + 2</param>
	/// <param name="t">Time between samples (0-1)</param>
	/// <returns></returns>
	float InterpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t) {
		float c0 = x1;
		float c1 = .5F * (x2 - x0);
		float c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
		float c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
		return (((((c3 * t) + c2) * t) + c1) * t) + c0;
	}

	int GetAdjustedSamplePosition(const MOISE_SamplePlayer_Sample &sampleData, int position) {
		if (startedLoopingSample) {
			if (position < sampleData.loopStart) {
				position += (sampleData.loopEnd - sampleData.loopStart);
			} else if (position >= sampleData.loopEnd) {
				position -= (sampleData.loopEnd - sampleData.loopStart);
			}
		}

		while (position < 0) {
			position += sampleData.waveformLength;
		}

		while (position >= sampleData.waveformLength) {
			position -= sampleData.waveformLength;
		}

		return position;
	}

	float GetInterpolatedSampleAtCurrentPosition(const MOISE_SamplePlayer_Sample &sampleData) {

		int samplePosition[4];

		//Get the current base sample position for interpolation by flooring the current precise position (with modulo to ensure position is within sample waveform bounds).
		int baseSamplePosition = static_cast<int>(std::floor(currentPosition)) % sampleData.waveform.size();
		baseSamplePosition = max(1, baseSamplePosition);

		for (int i = -1; i < 3; i++) {
			samplePosition[i + 1] = GetAdjustedSamplePosition(sampleData, baseSamplePosition + i);
		}

		if (samplePosition[0] >= sampleData.loopStart) {
			startedLoopingSample = true;
		}

		return InterpolateHermite4pt3oX(sampleData.waveform[samplePosition[0]],
			sampleData.waveform[samplePosition[1]],
			sampleData.waveform[samplePosition[2]],
			sampleData.waveform[samplePosition[3]],
			currentPosition - baseSamplePosition);
	}

	/// <summary>
	/// Get the next sample based on amount of time to advance.
	/// </summary>
	/// <param name="timeAdvance">The amount of time to advance.</param>
	/// <returns>A pointer to the next sample</returns>
	virtual float* GetNextSample(double timeAdvance) override {
		if (sampleBank.size() < 1) {
			return nullptr;
		}

		UpdateEnvelope(timeAdvance);

		if (active) {

			//Get active sample data
			const MOISE_SamplePlayer_Sample &sampleData = sampleBank[activeSampleId];

			//Get a sample from the waveform data at the currentSamplePosition.
			float centerSample = GetInterpolatedSampleAtCurrentPosition(sampleData);

			//Multiply the sample by the current envelope value
			centerSample *= currentEnvelope.currentValue;

			//Apply the sample to all channels. This is where panning will occur once implemented.
			for (int i = 0; i < channels; i++) {
				currentSample[i] = centerSample;
			}

			//Advance the current position based on the frequency being played, the rate, and the sample's root frequency.
			currentPosition += (sampleData.waveformRate / sampleRate) * (frequency / sampleData.rootFrequency);
			while (currentPosition >= sampleData.loopEnd &&
				sampleData.loopStart < sampleData.loopEnd) { //Prevent infinite loop in the case that loopStart >= loopEnd
				//Smoothly loop the sample back to the loop start point.
				currentPosition -= (sampleData.loopEnd - sampleData.loopStart);
			}
		}
		else {
			for (int i = 0; i < channels; i++) {
				currentSample[i] = 0;
			}
		}

		//Returns the current sample.
		return currentSample;
	}

	/// <summary>
	/// Activates the sample player voice and sets the note it's playing to the given value, while also selecting the appropriate sample based on the value.
	/// </summary>
	/// <param name="value"></param>
	virtual void NoteOn(int value) override {
		MoiseSynth::NoteOn(value);

		//Update the sample being used based on the current frequency
		for (int i = 0; i < sampleBank.size(); i++) {
			activeSampleId = i;

			if (sampleBank[i].minFrequency < frequency && sampleBank[i].maxFrequency >= frequency) {
				break;
			}
		}

		active = true;
		playing = true;
		currentEnvelope.state = EnvelopeState::Attacking;
		currentEnvelope.currentValue = 0;
		startedLoopingSample = false;
	}
};

#endif // !MOISE_SYNTH_SAMPLEPLAYER_H
