#pragma once

#ifdef DLLPROJECT_EXPORTS
#   define EXPORT __declspec(dllexport)
#else
#   define EXPORT __declspec(dllimport)
#endif

extern "C" EXPORT void Init(int setSampleRate);

extern "C" EXPORT void FillWaveformData(float data[], int sampleTotal, int channels);