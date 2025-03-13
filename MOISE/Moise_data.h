#pragma once

struct command {
	int tick;
	int function;
	int parameter;
};

struct track {
	command commands[0];
};