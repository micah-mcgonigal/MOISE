#pragma once

struct Command {
	int tick;
	int function;
	int parameter;
};

struct Track {
	Command commands[0];
};