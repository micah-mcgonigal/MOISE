#pragma once
#include "MoiseSynth.h"

#ifndef MOISE_SYNTH_SAMPLEPLAYER_H
#define MOISE_SYNTH_SAMPLEPLAYER_H

struct MOISE_SamplePlayer_Sample {
	float* waveform;
	float rootFrequency;
	float waveformRate;
	float positionRate;
	int loopStart;
	int loopEnd;
	//MOISE_Note minNote;
	//MOISE_Note maxNote;
};

struct MOISE_SamplePlayer_SampleBank {
	MOISE_SamplePlayer_Sample sample[1];
};

class MoiseSynth_SamplePlayer : public MoiseSynth {

private:
	MOISE_SamplePlayer_Sample sampleData;

public:
	int activeSampleId = 0; //The ID of the sample to use.

	MoiseSynth_SamplePlayer(MOISE_SamplePlayer_Sample* data, int waveformSampleCount) {
		sampleData = *data;
		
		//for (int i = 0; i < waveformSampleCount; i++) {
		//	sampleData.waveform[i] = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - -1)));
		//}
	}

	void Initialize(int setSampleRate, int setChannels) {
		sampleRate = setSampleRate;
		channels = setChannels;
		initialized = true;
	}

	float GetWaveformValue(int sampleIndex) override{
		return sampleData.waveform[sampleIndex];
	}

	virtual float* GetNextSample(double timeAdvance) override{
		//for (int i = 0; i < channels; i++) {
		//	currentSample[i] = std::sin(2 * 3.14159265358979323846 * 440 * currentTime);
		//}
		//
		//currentTime += timeAdvance;
		//
		//return currentSample;

		currentSamplePosition = std::floor(currentPosition);

		float centerSample = sampleData.waveform[currentSamplePosition];

		for (int i = 0; i < channels; i++) {
			currentSample[i] = centerSample;
		}

		currentPosition += sampleData.positionRate * (frequency / sampleData.rootFrequency);
		while (currentPosition >= sampleData.loopEnd) {
			currentPosition -= (sampleData.loopEnd - sampleData.loopStart);
		}

		return currentSample;
	}

	/*
	{ Old unity version for reference
            envelope.update();
            currentSamplePosition = Mathf.FloorToInt(currentPosition);

            float centerSample = sample[currentSample].DataAt(currentSamplePosition) * envelope.Value;
            leftSample = centerSample * Mathf.Cos(pan * (Mathf.PI / 2));
            rightSample = centerSample * Mathf.Sin(pan * (Mathf.PI / 2));

            currentPosition += sample[currentSample].PositionRate * (frequency / sample[currentSample].rootFrequency);

            if (currentPosition >= sample[currentSample].loopEnd)
            {
                currentPosition = sample[currentSample].loopStart;
            }

        }
	*/
};

#endif // !MOISE_SYNTH_SAMPLEPLAYER_H
