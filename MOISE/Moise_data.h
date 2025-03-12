#pragma once

#ifdef DLLPROJECT_EXPORTS
#   define EXPORT __declspec(dllexport)
#else
#   define EXPORT __declspec(dllimport)
#endif

struct command {
	int tick;
	int function;
	int parameter;
};

struct track {
	command commands[0];
};

extern "C" EXPORT int Test();