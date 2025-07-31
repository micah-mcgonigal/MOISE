#define no_init_all deprecated

#include "pch.h"
#include "Moise_data.h"
#include "MoiseSynth_SamplePlayer.h"
#include "MoiseLibrary.h"
#include <iostream>
#include <queue>

float MoiseSynth::stepHz = _CMATH_::pow(2.0, 1.0 / 12.0);

MoiseSynth* testSynth;

std::queue<Command> commandQueue;

Composition currentComposition;

double currentTime, preciseTick;
double timeAdvance;
int sampleRate, currentTick, lastTick;

int main() {
	return 0;
}

void Init(int setSampleRate) {
	sampleRate = setSampleRate;
	timeAdvance = 1.0 / sampleRate;
	currentTime = 0;

	//Hard coding a composition for testing purposes
	currentComposition.track = new Track[1];
	currentComposition.track[0].command = new Command[8];

	currentComposition.track[0].command[0].function = 1;
	currentComposition.track[0].command[0].parameter = 0;
	currentComposition.track[0].command[0].tick = 0;

	currentComposition.track[0].command[1].function = 1;
	currentComposition.track[0].command[1].parameter = 1*8;
	currentComposition.track[0].command[1].tick = 1*64;

	currentComposition.track[0].command[2].function = 1;
	currentComposition.track[0].command[2].parameter = 2 * 8;
	currentComposition.track[0].command[2].tick = 2 * 64;

	currentComposition.track[0].command[3].function = 1;
	currentComposition.track[0].command[3].parameter = 3 * 8;
	currentComposition.track[0].command[3].tick = 3 * 64;

	currentComposition.track[0].command[4].function = 1;
	currentComposition.track[0].command[4].parameter = 4 * 8;
	currentComposition.track[0].command[4].tick = 4 * 64;

	currentComposition.track[0].command[5].function = 1;
	currentComposition.track[0].command[5].parameter = 5 * 8;
	currentComposition.track[0].command[5].tick = 5 * 64;

	currentComposition.track[0].command[6].function = 1;
	currentComposition.track[0].command[6].parameter = 6 * 8;
	currentComposition.track[0].command[6].tick = 6 * 64;

	currentComposition.track[0].command[7].function = 1;
	currentComposition.track[0].command[7].parameter = 6 * 8;
	currentComposition.track[0].command[7].tick = (7 * 64) - 1;

	//testSynth->Initialize(sampleRate, 2);
}

int LoadPackage(Track* trackToLoad) {
	return trackToLoad->command[0].function;
}

float LoadSamplePlayerSynth(MOISE_SamplePlayer_Sample* data, int waveformSampleCount) {
	testSynth = new MoiseSynth_SamplePlayer(data, waveformSampleCount);
	testSynth->Initialize(sampleRate, 2);
	return testSynth->GetWaveformValue(16);
}

void ProcessCommand(Command command, int track) {
	switch (command.function) {
	case 0: // Note Off
		// Implement Note Off logic here
		break;
	case 1: // Note On
		testSynth->NoteOn(command.parameter);
		break;
	default:
		std::cerr << "Unknown command function: " << command.function << std::endl;
		break;
	}
}

int FillWaveformData(float data[], int sampleTotal, int channels) {

	for (int i = 0; i < sampleTotal; i += channels) {
		if (commandQueue.empty()) {
			commandQueue.push(currentComposition.track[0].command[0]);
			commandQueue.push(currentComposition.track[0].command[1]);
			commandQueue.push(currentComposition.track[0].command[2]);
			commandQueue.push(currentComposition.track[0].command[3]);
			commandQueue.push(currentComposition.track[0].command[4]);
			commandQueue.push(currentComposition.track[0].command[5]);
			commandQueue.push(currentComposition.track[0].command[6]);
			commandQueue.push(currentComposition.track[0].command[7]);
			preciseTick = 0.0;
		}

		//Update current tick and check for any new commands to process
		preciseTick += timeAdvance * 64; // Assuming 60 ticks per second for simplicity
		currentTick = std::floor(preciseTick);

		while (!commandQueue.empty() && commandQueue.front().tick <= currentTick) {
			Command currentCommand = commandQueue.front();
			commandQueue.pop();
			ProcessCommand(currentCommand, 0);
		}

		//Get waveform data for this sample
		float* newSample = testSynth->GetNextSample(timeAdvance);
		for (int j = 0; j < channels; j++) {
			data[i+j] = newSample[j];
		}
		currentTime += timeAdvance;
	}

	return currentTick;
}