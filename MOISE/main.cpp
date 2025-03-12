#define no_init_all deprecated

#include "pch.h"
#include "Moise_data.h"
#include <iostream>

int main() {
	track testTrack;
	testTrack.commands->tick = 99;
	std::cout << testTrack.commands->tick;

	return 0;
}

int Test() {
	return 1;
}