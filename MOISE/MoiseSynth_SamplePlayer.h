#pragma once
#include "MoiseSynth.h"

#ifndef MOISE_SYNTH_SAMPLEPLAYER_H
#define MOISE_SYNTH_SAMPLEPLAYER_H

class MoiseSynth_SamplePlayer : public MoiseSynth {

public:
	MoiseSynth_SamplePlayer() {

	}

	void Initialize(int setSampleRate, int setChannels) {
		sampleRate = setSampleRate;
		channels = setChannels;
		initialized = true;
	}

	virtual float* GetNextSample(double timeAdvance) {
		for (int i = 0; i < channels; i++) {
			currentSample[i] = std::sin(2 * 3.14159265358979323846 * 440 * currentTime);
		}

		currentTime += timeAdvance;

		return currentSample;
	}
};

struct MOISE_SamplePlayer_Sample {
	float waveform[1];
	float rootFrequency;
	float waveformRate;
	float positionRate;
	int loopStart;
	int loopEnd;
	MOISE_Note minNote;
	MOISE_Note maxNote;
};

struct MOISE_SamplePlayer_SampleBank {
	MOISE_SamplePlayer_Sample sample[1];
};

#endif // !MOISE_SYNTH_SAMPLEPLAYER_H
