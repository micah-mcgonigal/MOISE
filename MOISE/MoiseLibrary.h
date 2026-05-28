#pragma once

#ifdef DLLPROJECT_EXPORTS
#   define EXPORT __declspec(dllexport)
#else
#   define EXPORT __declspec(dllimport)
#endif

//Exporting functions for use via the .dll.

extern "C" EXPORT void Init(int setSampleRate);

extern "C" EXPORT int LoadPackage(Track* trackToLoad);

extern "C" EXPORT int LoadPackageFromFile(char* packagePath, char* sampleBankPath);

extern "C" EXPORT bool Play();

extern "C" EXPORT void Stop();

extern "C" EXPORT int FillWaveformData(float data[], int sampleTotal, int channels);

extern "C" EXPORT bool GetBool(const char *variableName);

extern "C" EXPORT bool SetBool(const char *variableName, bool newValue);

extern "C" EXPORT int GetInt(const char *variableName);

extern "C" EXPORT bool SetInt(const char *variableName, int newValue);

extern "C" EXPORT float GetFloat(const char *variableName);

extern "C" EXPORT bool SetFloat(const char *variableName, float newValue);

extern "C" EXPORT int GetNextCallbackInQueue(char *callback, int capacity);

