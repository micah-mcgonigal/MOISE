#define no_init_all deprecated

#include "pch.h"
#include "Moise_data.h"
#include "MoiseLibrary.h"
#include <iostream>

double currentTime;
double timeAdvance;
int sampleRate;

int main() {
	track testTrack;
	testTrack.commands->tick = 99;
	std::cout << testTrack.commands->tick;

	return 0;
}

void Init(int setSampleRate) {
	sampleRate = setSampleRate;
	timeAdvance = 1.0 / sampleRate;
	currentTime = 0;
}

double Test(float data[], int sampleTotal) {

	for (int i = 0; i < sampleTotal; i += 2) {
		data[i] = sin(2 * 3.14159265358979323846 * 440 * currentTime);
		data[i+1] = sin(2 * 3.14159265358979323846 * 440 * currentTime);
		currentTime += timeAdvance;
	}

	return currentTime;
}