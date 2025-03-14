#pragma once
#include <cmath>

#ifndef MOISE_SYNTH_H
#define MOISE_SYNTH_H

class MoiseSynth {
	static float stepHz;

private:
	float currentSample[1];

protected:
	double currentTime;
	float frequency = 440;
	float pan = 0.5;
	float currentPosition;
	float currentEnvelopePosition;
	float currentEnvelopeValue;
	int currentSamplePosition;
	int currentValue;
	int sampleRate;
	int channels;
	bool active;
	bool initialized;

public:
	MoiseSynth() {

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

#endif // !MOISE_SYNTH_H
