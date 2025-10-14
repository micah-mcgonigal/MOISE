#define no_init_all deprecated

#include "pch.h"
#include "Moise_data.h"
#include "MoiseSynth_SamplePlayer.h"
#include "MoiseLibrary.h"
#include <iostream>
#include <queue>

//stepHz is a constant used to calculate the frequency of a note based on its MOISE_Note value
float MoiseSynth::stepHz = _CMATH_::pow(2.0, 1.0 / 12.0);

//Pointer to a test synth instance
MoiseSynth* testSynth;

//A queue used to manage commands as loaded in each playing track.
std::queue<Command> commandQueue;

//The currently loaded composition.
Composition currentComposition;

double currentTime, preciseTick; //Currrent time in seconds and ticks. Ticks are to be determined by a ticks-per-measure value.
double timeAdvance; //Holds the amount of time to advance for each sample.
int sampleRate, currentTick, lastTick;

int main() {
	return 0;
}

void Init(int setSampleRate) {
	//The sample rate is set by the host application.
	sampleRate = setSampleRate;
	timeAdvance = 1.0 / sampleRate;
	currentTime = 0;

	//Hard coding a composition for testing purposes. This is why the editor is needed!
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

/// <summary>
/// This is the function that is to load a MOISE package, representing an entire MOISE project, into memory. Currently, for testing purposes, it is only set up to return the function value of the first command of a track.
/// </summary>
/// <param name="trackToLoad">Pointer of the track to check.</param>
/// <returns>The integer value of command[0]'s function.</returns>
int LoadPackage(Track* trackToLoad) {
	return trackToLoad->command[0].function;
}

/// <summary>
/// Loads a sample player MOISE synth with the provided sample data.
/// </summary>
/// <param name="data">Pointer to MOISE_SamplePlayer_Sample data, which determines the sample's waveform and meta data, such as loop points.</param>
/// <param name="waveformSampleCount">The sample count of the waveform. Note: I'm not sure why this isn't just included in MOISE_SamplePlayer_Sample.</param>
/// <returns></returns>
float LoadSamplePlayerSynth(MOISE_SamplePlayer_Sample* data, int waveformSampleCount) {
	testSynth = new MoiseSynth_SamplePlayer(data, waveformSampleCount);
	testSynth->Initialize(sampleRate, 2);
	return testSynth->GetWaveformValue(16); //Return a sample value for testing purposes.
}


float LoadSamplePlayerSynthFromFile(char* jsonPath, char* pcmRootPath) {
	testSynth = new MoiseSynth_SamplePlayer(jsonPath, pcmRootPath);
	testSynth->Initialize(sampleRate, 2);
	return testSynth->GetWaveformValue(16); //Return a sample value for testing purposes, to mirror LoadSamplePlayerSynth().
}

/// <summary>
/// Processes MOISE commands.
/// </summary>
/// <param name="command">The command.</param>
/// <param name="track">The track that the command is sent to.</param>
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

/// <summary>
/// This function fills the provided float array with the output waveform data. This function must be called by the host program as the audio buffer needs to be filled. This will provide waveform data while also advancing MOISE.
/// </summary>
/// <param name="data">The float array to be filled with output waveform data</param>
/// <param name="sampleTotal">The number of samples (The size of data[]) to fill</param>
/// <param name="channels">The number of audio channels being used (1 = mono, 2 = stereo)</param>
/// <returns></returns>
int FillWaveformData(float data[], int sampleTotal, int channels) {

	for (int i = 0; i < sampleTotal; i += channels) {

		//Hard coded queue filling for testing purposes.
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

		//Advance the current tick and check for any new commands to process
		preciseTick += timeAdvance * 64; // Assuming 60 ticks per second for simplicity NOTE: the "* 64" is to speed up the playback for testing purposes.
		currentTick = std::floor(preciseTick);

		//Processes any queued commands that are due to be executed based on the current tick.
		while (!commandQueue.empty() && commandQueue.front().tick <= currentTick) {
			Command currentCommand = commandQueue.front();
			commandQueue.pop();
			ProcessCommand(currentCommand, 0);
		}

		//Get waveform data from active synths for this sample. Currently, only one synth is supported for testing purposes. This is to be a loop through all active synths in the future.
		float* newSample = testSynth->GetNextSample(timeAdvance);
		for (int j = 0; j < channels; j++) {
			data[i+j] = newSample[j];
		}

		//Advance the current real time that has passed for this sample.
		currentTime += timeAdvance;
	}

	//Returns the current tick in order to easily view the progression of time in the MOISE editor.
	return currentTick;
}