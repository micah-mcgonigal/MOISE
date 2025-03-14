#define no_init_all deprecated

#include "pch.h"
#include "Moise_data.h"
#include "MoiseLibrary.h"
#include "json.hpp"
#include <string>
#include <iostream>

using json = nlohmann::json;

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

int LoadPackage(std::string jsonString) {
	json data = json::parse(jsonString);

	track testTrack = data.template get<track>();
}

void FillWaveformData(float data[], int sampleTotal, int channels) {

	for (int i = 0; i < sampleTotal; i += 2) {
		for (int j = 0; j < channels; j++) {
			data[i+j] = sin(2 * 3.14159265358979323846 * 440 * currentTime);
		}
		currentTime += timeAdvance;
	}
}