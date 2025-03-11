#include <iostream>

struct command {
	int tick;
	int function;
	int parameter;
};

struct track {
	command commands[0];
};

main() {
	track testTrack;
	testTrack.commands->tick = 99;
	std::cout << testTrack.commands->tick;
}

