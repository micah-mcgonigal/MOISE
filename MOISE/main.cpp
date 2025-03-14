#define no_init_all deprecated

#include "pch.h"
#include "Moise_data.h"
#include "MoiseLibrary.h"
#include "MoiseSynth_SamplePlayer.h"
#include <iostream>

float MoiseSynth::stepHz = _CMATH_::pow(2.0, 1.0 / 12.0);

MoiseSynth testSynth;

double currentTime;
double timeAdvance;
int sampleRate;


int main() {
	Track testTrack;
	testTrack.commands->tick = 99;
	std::cout << testTrack.commands->tick;

	return 0;
}

void Init(int setSampleRate) {
	sampleRate = setSampleRate;
	timeAdvance = 1.0 / sampleRate;
	currentTime = 0;

	testSynth = MoiseSynth_SamplePlayer();
	testSynth.Initialize(sampleRate, 2);
}

int LoadPackage(Track* trackToLoad) {
	return trackToLoad->commands[0].function;
}

void FillWaveformData(float data[], int sampleTotal, int channels) {

	for (int i = 0; i < sampleTotal; i += channels) {
		float* newSample = testSynth.GetNextSample(timeAdvance);
		for (int j = 0; j < channels; j++) {
			data[i+j] = newSample[j];
		}
		currentTime += timeAdvance;
	}
}