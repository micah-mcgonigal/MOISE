#pragma once
#include <cmath>

#ifndef MOISE_SYNTH_H
#define MOISE_SYNTH_H

class MoiseSynth {
	static float stepHz;

protected:
	double currentTime;
	float currentSample[2];
	float frequency = 440;
	float pan = 0.5;
	double currentPosition = 0;
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
		stepHz = std::pow(2.0, 1.0 / 12.0); 
		initialized = true;
	}

	virtual float GetWaveformValue(int sampleIndex) {
		return -1;
	}

	virtual float* GetNextSample(double timeAdvance) {
		for (int i = 0; i < channels; i++) {
			currentSample[i] = std::sin(2 * 3.14159265358979323846 * frequency * currentTime);
		}

		currentTime += timeAdvance;
		
		return currentSample;
	}

	virtual void NoteOn(int value) {
		active = true;
		frequency = 440 * std::pow(stepHz, value);
		currentValue = value;
		currentPosition = 0;
	}
};

#endif // !MOISE_SYNTH_H
