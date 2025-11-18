#define no_init_all deprecated

#include "pch.h"
#include "Moise_data.h"
#include "MoiseSynth_SamplePlayer.h"
#include "MoiseLibrary.h"
#include <iostream>
#include <queue>

//stepHz is a constant used to calculate the frequency of a note based on its MOISE_Note value
float MoiseSynth::stepHz = _CMATH_::pow(2.0, 1.0 / 12.0);

//Loaded synth instances
std::vector<std::unique_ptr<MoiseSynth>> synths = {};

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

	//Clear all synths
	synths.clear();
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
/// Load a package from file paths (package file + sample bank file).
/// </summary>
/// <param name="packagePath">The path to the packakge file</param>
/// <param name="sampleBankPath">The path to the sample bank file</param>
/// <returns>0 if no errors occurred while loading, 1 if errors occurred</returns>
int LoadPackageFromFile(char* packagePath, char* sampleBankPath) {
	// --------
	// Sample bank (from binary)

	// TODO: Use std::filesystem::path::preferred_separator (requires C++17)
#ifdef _WIN32
	static char pathSeparator = '\\';
#else
	static char pathSeparator = '/';
#endif

	// Open sample bank file
	std::ifstream sampleBankIn;
	sampleBankIn.open(sampleBankPath, std::ios::binary);
	if (!sampleBankIn.good()) {
		std::cerr << "LoadPackageFromFile: Invalid sample bank path " << sampleBankPath << std::endl;
		return 1;
	}

	// --------
	// Package metadata (from JSON)

	// Load JSON data from file
	std::ifstream jsonIn(packagePath);
	if (!json::accept(jsonIn)) {
		std::cerr << "LoadPackageFromFile: Couldn't load JSON data at " << packagePath << std::endl;
		return 1;
	}
	jsonIn.seekg(0, std::ios::beg);
	json jsonData = json::parse(jsonIn);

	// Get synth data from the package JSON
	if (!jsonData.contains("instruments")) {
		std::cerr << "LoadPackageFromFile: No instruments/synths contained in JSON data at " << packagePath << std::endl;
		return 1;
	}

	// --------
	// Synths/instruments (using both package data and sample bank)

	std::vector<json> instrumentsJson = jsonData.at("instruments").get<std::vector<json>>();

	for (const auto& instrumentJson : instrumentsJson) {
		// Get default envelope
		if (!instrumentJson.contains("defaultEnvelope")) {
			std::cerr << "LoadPackageFromFile: No default envelope found in instrument " << instrumentJson.dump() << std::endl;
			continue;
		}
		Envelope defaultEnvelope = instrumentJson.at("defaultEnvelope").get<Envelope>();

		// Get sample bank
		if (!instrumentJson.contains("sampleBank")) {
			std::cerr << "LoadPackageFromFile: No sample bank found in instrument " << instrumentJson.dump() << std::endl;
			continue;
		}
		std::vector<json> sampleBankJson = instrumentJson.at("sampleBank").get<std::vector<json>>();

		if (sampleBankJson.size() < 1) {
			std::cout << "LoadPackageFromFile: Sample bank is empty for instrument " << instrumentJson.dump() << std::endl;
			continue;
		}

		// Create new sample player synth for instrument
		synths.push_back(std::make_unique<MoiseSynth_SamplePlayer>());
		MoiseSynth_SamplePlayer* samplePlayerSynth = static_cast<MoiseSynth_SamplePlayer*>(synths.back().get());
		if (!samplePlayerSynth) {
			std::cerr << "LoadPackageFromFile: Unable to create sample player for instrument " << instrumentJson.dump() << std::endl;
			continue;
		}

		// Set default envelope
		samplePlayerSynth->SetDefaultEnvelope(defaultEnvelope);

		// Load samples into bank
		for (const auto& sampleJson : sampleBankJson) {
			MOISE_SamplePlayer_Sample sampleData = sampleJson.get<MOISE_SamplePlayer_Sample>();
			samplePlayerSynth->LoadSampleIntoBank(sampleData, sampleBankIn);
		}

		// Initialize sample player
		samplePlayerSynth->Initialize(sampleRate, 2);
	}

	// --------
	// Test synth

	// TODO: Assign test synth more meaningfully -- currently we just take the first available synth
	if (synths.size() > 0) {
		testSynth = synths[0].get();
	}
	else {
		std::cout << "LoadPackageFromFile: No synths loaded!" << std::endl;
	}

	// --------

	return 0;
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

/// <summary>
/// Get the test synth's first sample's waveform value at a specific sample index.
/// </summary>
/// <param name="sampleIndex">The sample index</param>
/// <returns>The waveform value</returns>
float GetTestSynthWaveformValue(int sampleIndex) {
	if (!testSynth) {
		return -1.f;
	}

	return testSynth->GetWaveformValue(sampleIndex);
}
