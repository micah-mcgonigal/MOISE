#pragma once

#ifdef DLLPROJECT_EXPORTS
#   define EXPORT __declspec(dllexport)
#else
#   define EXPORT __declspec(dllimport)
#endif

//Exporting functions for use via the .dll.

extern "C" EXPORT void Init(int setSampleRate);

extern "C" EXPORT int LoadPackage(Track* trackToLoad);

extern "C" EXPORT int FillWaveformData(float data[], int sampleTotal, int channels);

extern "C" EXPORT float LoadSamplePlayerSynth(MOISE_SamplePlayer_Sample* data, int waveformSampleCount);