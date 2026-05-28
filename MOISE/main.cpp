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

//The currently loaded composition.
Composition currentComposition;

//Variables are used for processing command conditions.
std::unordered_map<std::string, Variable> variables;

//Queue of strings for callbacks
std::queue<std::string> callbackQueue;

double currentTime, preciseTick; //Currrent time in seconds and ticks. Ticks are to be determined by a ticks-per-measure value.
double timeAdvance; //Holds the amount of time to advance for each sample.
std::vector<int> currentCommandIndexPerTrack;
int sampleRate, currentTick = -1, lastTick; //Current tick should be negative one when a song is stopped or changed.
bool playing = false, compositionReady = false;

int main() {
	return 0;
}

void Init(int setSampleRate) {
	//The sample rate is set by the host application.
	sampleRate = setSampleRate;
	timeAdvance = 1.0 / sampleRate;
	currentTime = 0;
	currentCommandIndexPerTrack.resize(64); //Just default to 64 tracks for flexibility

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
	return trackToLoad->commands[0].function;
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
	if (synths.size() <= 0) {
		std::cout << "LoadPackageFromFile: No synths loaded!" << std::endl;
	}

	// --------
	//Load composition data from the package JSON
	if (!jsonData.contains("songs")) {
		std::cerr << "LoadPackageFromFile: No songs contained in JSON data at " << packagePath << std::endl;
		return 1;
	}

	std::vector<json> songsJson = jsonData.at("songs").get<std::vector<json>>();

	for (const auto &songJson : songsJson) { //Load all songs in the package
		// For testing purposes, we will just load the first composition of the first song
		if (!songJson.contains("compositions")) {
			std::cerr << "LoadPackageFromFile: No compositions contained in song " << songJson.dump() << std::endl;
			continue;
		}

		std::vector<json> compositionsJson = songJson.at("compositions").get<std::vector<json>>();

		for (const auto &compositionJson : compositionsJson) {
			if (!compositionJson.contains("tracks")) {
				std::cerr << "LoadPackageFromFile: No tracks contained in composition " << compositionJson.dump() << std::endl;
				continue;
			}

			currentComposition = compositionJson.get<Composition>();
			compositionReady = true;
		}

	}

	// --------
	//Load variable data from the package JSON
	if (!jsonData.contains("variables")) {
		std::cerr << "LoadPackageFromFile: No variables contained in JSON data at " << packagePath << std::endl;
	}
	else {
		std::vector<json> variablesJson = jsonData.at("variables").get<std::vector<json>>();

		for (const auto &variableJson : variablesJson) {
			variables.insert({ variableJson.at("name").get<std::string>(), variableJson.get<Variable>() });
		}
	}

	return 0;
}

bool Play() {
	if (!compositionReady) {
		std::cerr << "No composition is loaded and so playback cannot begin." << std::endl;
		return false;
	}
	playing = true;
	return true;
}

void Stop() {
	playing = false;
	preciseTick = 0;
	currentTick = -1;
}

int GetNextCallbackInQueue(char *callback, int capacity) {
	if (callbackQueue.size() > 0) {
		if (callbackQueue.front().length() > capacity) {
			std::cerr << "Callback string was larger than the available capacity." << std::endl;
			return 0;
		}
		strcpy_s(callback, capacity, callbackQueue.front().c_str());
		callbackQueue.pop();
		return callbackQueue.size() + 1;
	}

	return 0;
}

//I know there's probably a better way to handle this but this is the best I can come up with right now.
bool CompareBools(Variable variable, Condition condition) {
	switch (condition.comparisonType) {
	case 0:
		return variable.boolValue == condition.boolValue;
		break;
	case 1:
		return variable.boolValue != condition.boolValue;
		break;
	default:
		return false;
		std::cerr << "CompareBools: comparisonType is invalid. comparisonType is " << variable.type << std::endl;
		break;
	}

	return false;
}

bool CompareInts(Variable variable, Condition condition) {
	switch (condition.comparisonType) {
	case 0:
		return variable.intValue == condition.intValue;
		break;
	case 1:
		return variable.intValue != condition.intValue;
		break;
	case 2:
		return variable.intValue < condition.intValue;
		break;
	case 3:
		return variable.intValue > condition.intValue;
		break;
	case 4:
		return variable.intValue <= condition.intValue;
		break;
	case 5:
		return variable.intValue >= condition.intValue;
		break;
	default:
		std::cerr << "CompareInts: comparisonType is invalid. comparisonType is " << variable.type << std::endl;
		return false;
	}

	return false;
}

bool CompareFloats(Variable variable, Condition condition) {
	switch (condition.comparisonType) {
	case 0:
		return variable.floatValue == condition.floatValue;
		break;
	case 1:
		return variable.floatValue != condition.floatValue;
		break;
	case 2:
		return variable.floatValue < condition.floatValue;
		break;
	case 3:
		return variable.floatValue > condition.floatValue;
		break;
	case 4:
		return variable.floatValue <= condition.floatValue;
		break;
	case 5:
		return variable.floatValue >= condition.floatValue;
		break;
	default:
		std::cerr << "CompareFloats: comparisonType is invalid. comparisonType is " << variable.type << std::endl;
		return false;
	}

	return false;
}

bool ProcessCondition(Condition condition) {
	auto it = variables.find(condition.variableName);
	if (it == variables.end()) {
		// variable not found
		std::cerr << "Variable not found: " << condition.variableName << std::endl;
		return false;
	}

	Variable variable = it->second;

	switch (variable.type) {
	case 0: //Bool
		return CompareBools(variable, condition);
		break;
	case 1: //Int
		return CompareInts(variable, condition);
		break;
	case 2: //Float
		return CompareFloats(variable, condition);
		break;
	default:
		std::cerr << "ProcessCondition: Variable type is invalid. VariableType is " << variable.type << std::endl;
		return false;
		break;
	}

	return false;
}

float GetFloatFromParameterSet(ParameterSet &set) {
	float result = 0;
	float operand = 0;

	for (int i = 0; i < set.parameters.size(); i++) {
		switch (set.parameters[i].type) {
		case 0:
			operand = set.parameters[i].intParameter;
			break;
		case 1:
			continue;
			operand = set.parameters[i].floatParameter;
			break;
		case 2:
			auto it = variables.find(set.parameters[i].variableParameter);
			if (it == variables.end()) {
				// variable not found
				std::cerr << "Parmaeter variable not found: " << set.parameters[i].variableParameter << std::endl;
				continue;
			}

			Variable variable = it->second;
			operand = variable.floatValue;
			break;
		}

		switch (set.parameters[i].parameterOperator) {
		case 0:
			result += operand;
			break;
		case 1:
			result -= operand;
			break;
		case 2:
			result *= operand;
			break;
		case 3:
			result = result / operand;
			break;
		case 4:
			result = pow(result, operand);
			break;
		}
	}

	return result;
}

int GetIntFromParameterSet(ParameterSet &set) {
	int result = 0;
	int operand = 0;

	for (int i = 0; i < set.parameters.size(); i++) {
		switch (set.parameters[i].type) {
		case 0:
			operand = set.parameters[i].intParameter;
			break;
		case 1:
			continue;
			break;

		case 2:
			auto it = variables.find(set.parameters[i].variableParameter);
			if (it == variables.end()) {
				// variable not found
				std::cerr << "Parmaeter variable not found: " << set.parameters[i].variableParameter << std::endl;
				continue;
			}

			Variable variable = it->second;
			if (variable.type == 1) {
				operand = variable.intValue;
			}
			else {
				continue;
			}
			break;
		}

		switch (set.parameters[i].parameterOperator) {
		case 0:
			result += operand;
			break;
		case 1:
			result -= operand;
			break;
		case 2:
			result *= operand;
			break;
		case 3:
			result = std::lround(result / operand);
			break;
		case 4:
			result = std::lround(pow(result, operand));
			break;
		}
	}

	return result;
}

/// <summary>
/// Processes MOISE commands.
/// </summary>
/// <param name="command">The command.</param>
/// <param name="track">The track that the command is sent to.</param>
bool ProcessCommand(Command command, int track) {
	//First see if the command's conditions are met.
	for (Condition condition : command.conditions)
	{
		if (!ProcessCondition(condition)) {
			return false;
		}
	}

	if (synths.size() <= track) {
		std::cerr << "ProcessCommand: No synth found for track " << track << std::endl;
		return false;
	}

	switch (command.function) {
	case 0: // Note Off
		synths[track]->NoteOff();
		break;
	case 1: // Note On
		if (command.parameterSets.size() > 0) {
			synths[track]->NoteOn(GetIntFromParameterSet(command.parameterSets[0]));
		}
		break;
	case 3: // Callback
		if (command.parameterSets.size() > 0 && command.parameterSets[0].parameters.size() > 0) {
			callbackQueue.push(command.parameterSets[0].parameters[0].variableParameter);
		}
		break;
	default:
		std::cerr << "Unknown command function: " << command.function << std::endl;
		break;
	}

	return true;
}
 //This should be updated to send the bool via an out so that it can return based on whether or not it was successful
bool GetBool(const char *variableName) {
	auto it = variables.find(variableName);
	if (it == variables.end()) {
		// variable not found
		std::cerr << "GetBool: Variable not found: " << variableName << std::endl;
		return false;
	}

	if (it->second.type != 0) {
		// variable not correct type
		std::cerr << "GetBool: Variable is not a bool. Actual type = " << it->second.type << std::endl;
		return false;
	}

	return it->second.boolValue;
}

bool SetBool(const char *variableName, bool newValue) {
	auto it = variables.find(variableName);
	if (it == variables.end()) {
		// variable not found
		std::cerr << "SetBool: Variable not found: " << variableName << std::endl;
		return false;
	}

	if (it->second.type != 0) {
		// variable not correct type
		std::cerr << "SetBool: Variable is not a bool. Actual type = " << it->second.type << std::endl;
		return false;
	}

	it->second.boolValue = newValue;
	return true;
}

int GetInt(const char *variableName) {
	auto it = variables.find(variableName);
	if (it == variables.end()) {
		// variable not found
		std::cerr << "GetInt: Variable not found: " << variableName << std::endl;
		return 0;
	}

	if (it->second.type != 1) {
		// variable not correct type
		std::cerr << "GetInt: Variable is not an int. Actual type = " << it->second.type << std::endl;
		return 0;
	}

	return it->second.intValue;
}

bool SetInt(const char *variableName, int newValue) {
	auto it = variables.find(variableName);
	if (it == variables.end()) {
		// variable not found
		std::cerr << "SetInt: Variable not found: " << variableName << std::endl;
		return false;
	}

	if (it->second.type != 1) {
		// variable not correct type
		std::cerr << "SetInt: Variable is not an int. Actual type = " << it->second.type << std::endl;
		return false;
	}

	it->second.intValue = newValue;
	return true;
}

float GetFloat(const char *variableName) {
	auto it = variables.find(variableName);
	if (it == variables.end()) {
		// variable not found
		std::cerr << "GetFloat: Variable not found: " << variableName << std::endl;
		return 0;
	}

	if (it->second.type != 2) {
		// variable not correct type
		std::cerr << "GetFloat: Variable is not a float. Actual type = " << it->second.type << std::endl;
		return 0;
	}

	return it->second.floatValue;
}

bool SetFloat(const char *variableName, float newValue) {
	auto it = variables.find(variableName);
	if (it == variables.end()) {
		// variable not found
		std::cerr << "SetFloat: Variable not found: " << variableName << std::endl;
		return false;
	}

	if (it->second.type != 2) {
		// variable not correct type
		std::cerr << "SetFloat: Variable is not a float. Actual type = " << it->second.type << std::endl;
		return false;
	}

	it->second.floatValue = newValue;
	return true;
}

/// <summary>
/// This function fills the provided float array with the output waveform data. This function must be called by the host program as the audio buffer needs to be filled. This will provide waveform data while also advancing MOISE.
/// </summary>
/// <param name="data">The float array to be filled with output waveform data</param>
/// <param name="sampleTotal">The number of samples (The size of data[]) to fill</param>
/// <param name="channels">The number of audio channels being used (1 = mono, 2 = stereo)</param>
/// <returns></returns>
int FillWaveformData(float data[], int sampleTotal, int channels) {
	if (playing) {
		for (int i = 0; i < sampleTotal; i += channels) {

			//Advance the current tick and check for any new commands to process
			preciseTick += timeAdvance * 64; // Assuming 60 ticks per second for simplicity NOTE: the "* 64" is to speed up the playback for testing purposes.

			//Advance one tick at a time to make sure all commands are processed
			while (currentTick < std::floor(preciseTick)) {
				currentTick += 1;

				//If we have gone beyond the current composition's total ticks, reset ticks to the beginning
				if (currentTick >= currentComposition.totalTicks) {
					preciseTick -= currentComposition.totalTicks;
					currentTick = 0;

					for (int t = 0; t < currentCommandIndexPerTrack.size(); t++) {
						currentCommandIndexPerTrack[t] = 0;
					}
				}

				//Process all the commands for this tick across all tracks as well as any that may have been missed somehow (That shouldn't be possible, but better safe than sorry)
				for (int t = 0; t < currentComposition.tracks.size(); t++) {
					for (int c = currentCommandIndexPerTrack[t]; c < currentComposition.tracks[t].commands.size() && currentComposition.tracks[t].commands[c].tick <= currentTick; c++) {
						ProcessCommand(currentComposition.tracks[t].commands[c], t);
						currentCommandIndexPerTrack[t] = c + 1;
					}
				}
			}

			//Get waveform data from active synths for this sample. Currently, only one synth is supported for testing purposes. This is to be a loop through all active synths in the future.
			if (synths.size() > 0) {
				float *newSample = synths[0]->GetNextSample(timeAdvance);
				for (int j = 0; j < channels; j++) {
					data[i + j] = newSample[j];
				}
			}

			//Advance the current real time that has passed for this sample.
			currentTime += timeAdvance;
		}
	}

	//Returns the current tick in order to easily view the progression of time in the MOISE editor.
	return currentTick;
}
