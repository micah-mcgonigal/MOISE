#pragma once
#include "MoiseSynth.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"

#ifndef MOISE_SYNTH_SAMPLEPLAYER_H
#define MOISE_SYNTH_SAMPLEPLAYER_H

using json = nlohmann::json;

struct MOISE_SamplePlayer_Sample {
	float *waveform; //Pointer to an array of floats representing the waveform of the sample.
	float rootFrequency; //The frequency at which the sample was recorded.
	float waveformRate; //The base sample rate of the waveform.
	float positionRate; //The adjusted sample rate of the waveform based on the current sample rate of the MOISE engine and the waveformRate.
	int loopStart; //The sample index at which the sample's loop starts.
	int loopEnd; //The sample index at which the sample's loop ends.
	//MOISE_Note minNote; //The minimum MOISE_Note value that can trigger this sample.
	//MOISE_Note maxNote; //The maximum MOISE_Note value that can trigger this sample.
	int waveformLength; //The length of the waveform (in samples).
	std::string waveformPath; //The relative path to the waveform, relative to the samples root path.
};

// This allows us to deserialize JSON directly into our MOISE_SamplePlayer_Sample struct
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	MOISE_SamplePlayer_Sample,
	rootFrequency,
	waveformRate,
	positionRate,
	loopStart,
	loopEnd,
	waveformLength,
	waveformPath)

/// <summary>
/// A sample bank represents a collection of samples that can be used depending on the note being played. For example, a piano sample bank may contain different samples for each octave of the piano keyboard.
/// This is not yet implemented.
/// </summary>
struct MOISE_SamplePlayer_SampleBank {
	MOISE_SamplePlayer_Sample sample[1];
};

class MoiseSynth_SamplePlayer : public MoiseSynth {

private:
	MOISE_SamplePlayer_Sample sampleData; //Sample data for this player.
	std::vector<float> waveform; //Buffer of floats that holds the waveform data for this sample.

public:
	int activeSampleId = 0; //The ID of the sample to use for when sample banks are in use.

	/// <summary>
	/// Loads sample data from memory when creating an instance of the sample player synth.
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
	/// Loads sample data from a JSON file when creating an instance of the sample player synth.
	/// NOTE: Currently this JSON file represents a track, which then holds a sample (along with a path to the waveform
	///       itself) -- eventually this file will likely represent an entire package, in which case the JSON parsing
	///       can happen on the package level, and this MoiseSynth_SamplePlayer constructor can be rewritten to accept
	///       a JSON object instead of a JSON file path. We may also want to decouple samples from tracks, depending on
	///       the use cases.
	/// </summary>
	/// <param name="jsonPath">The path to the JSON file that holds the sample data</param>
	/// <param name="waveformRootPath">The root path for waveform data</param>
	MoiseSynth_SamplePlayer(char* jsonPath, char* waveformRootPath) {
		// --------
		// Sample metadata (from JSON)

		// Load JSON data from file
		std::ifstream jsonIn(jsonPath);
		if (!json::accept(jsonIn)) {
			std::cerr << "MoiseSynth_SamplePlayer: Couldn't load JSON data at " << jsonPath << std::endl;
			return;
		}
		jsonIn.seekg(0, std::ios::beg);
		json jsonData = json::parse(jsonIn);

		// Get sample data from the track JSON
		if (!jsonData.contains("sample")) {
			std::cerr << "MoiseSynth_SamplePlayer: No sample contained in JSON data at " << jsonPath << std::endl;
			return;
		}
		json sampleDataJson = jsonData.at("sample");
		sampleData = sampleDataJson.get<MOISE_SamplePlayer_Sample>();

		// --------
		// Sample waveform (from binary)

		// TODO: Use std::filesystem::path::preferred_separator (requires C++17)
#ifdef _WIN32
		static char pathSeparator = '\\';
#else
		static char pathSeparator = '/';
#endif

		// Construct full path to waveform file
		std::ifstream waveformIn;
		std::string waveformPathString = std::string(waveformRootPath);
		waveformPathString += pathSeparator + sampleData.waveformPath;
		const char* waveformPath = waveformPathString.c_str();

		// Open waveform file
		waveformIn.open(waveformPath, std::ios::binary);
		if (!waveformIn.good()) {
			std::cerr << "MoiseSynth_SamplePlayer: Invalid sample waveform path " << waveformPath << std::endl;
			return;
		}

		// Resize waveform buffer
		waveform.resize(sampleData.waveformLength);

		// Load waveform file's data into our waveform buffer
		if (!waveformIn.read(reinterpret_cast<char*>(waveform.data()), sampleData.waveformLength * sizeof(float))) {
			std::cerr << "MoiseSynth_SamplePlayer: Failed to load sample waveform data at " << waveformPath << std::endl;
		}

		// Set our sample data's waveform to point directly to our own waveform buffer
		sampleData.waveform = waveform.data();
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
