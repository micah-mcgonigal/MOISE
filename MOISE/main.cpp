#define no_init_all deprecated

#include "pch.h"
#include "Moise_data.h"
#include "MoiseLibrary.h"
#include <iostream>

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
}

int LoadPackage(Track* trackToLoad) {
	return trackToLoad->commands[0].function;
}

void FillWaveformData(float data[], int sampleTotal, int channels) {

	for (int i = 0; i < sampleTotal; i += 2) {
		for (int j = 0; j < channels; j++) {
			data[i+j] = sin(2 * 3.14159265358979323846 * 440 * currentTime);
		}
		currentTime += timeAdvance;
	}
}