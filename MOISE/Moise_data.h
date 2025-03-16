#pragma once

struct Command {
	int tick;
	int function;
	int parameter;
};

struct Track {
	Command commands[0];
};

enum MOISE_Note
{
	c0 = -57,
	c0s = -56,
	d0 = -55,
	d0s = -54,
	e0 = -53,
	f0 = -52,
	f0s = -51,
	g0 = -50,
	g0s = -49,
	a0 = -48,
	a0s = -47,
	b0 = -46,
	c1 = -45,
	c1s = -44,
	d1 = -43,
	d1s = -42,
	e1 = -41,
	f1 = -40,
	f1s = -39,
	g1 = -38,
	g1s = -37,
	a1 = -36,
	a1s = -35,
	b1 = -34,
	c2 = -33,
	c2s = -32,
	d2 = -31,
	d2s = -30,
	e2 = -29,
	f2 = -28,
	f2s = -27,
	g2 = -26,
	g2s = -25,
	a2 = -24,
	a2s = -23,
	b2 = -22,
	c3 = -21,
	c3s = -20,
	d3 = -19,
	d3s = -18,
	e3 = -17,
	f3 = -16,
	f3s = -15,
	g3 = -14,
	g3s = -13,
	a3 = -12,
	a3s = -11,
	b3 = -10,
	c4 = -9,
	c4s = -8,
	d4 = -7,
	d4s = -6,
	e4 = -5,
	f4 = -4,
	f4s = -3,
	g4 = -2,
	g4s = -1,
	a4 = 0,
	a4s = 1,
	b4 = 2,
	c5 = 3,
	c5s = 4,
	d5 = 5,
	d5s = 6,
	e5 = 7,
	f5 = 8,
	f5s = 9,
	g5 = 10,
	g5s = 11,
	a5 = 12,
	a5s = 13,
	b5 = 14,
	c6 = 15,
	c6s = 16,
	d6 = 17,
	d6s = 18,
	e6 = 19,
	f6 = 20,
	f6s = 21,
	g6 = 22,
	g6s = 23,
	a6 = 24,
	a6s = 25,
	b6 = 26,
	c7 = 27,
	c7s = 28,
	d7 = 29,
	d7s = 30,
	e7 = 31,
	f7 = 32,
	f7s = 33,
	g7 = 34,
	g7s = 35,
	a7 = 36,
	a7s = 37,
	b7 = 38,
	c8 = 39,
	c8s = 40,
	d8 = 41,
	d8s = 42,
	e8 = 43,
	f8 = 44,
	f8s = 45,
	g8 = 46,
	g8s = 47,
	a8 = 48
};