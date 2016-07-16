//============================================================================
// Name        : 8chipemulator.cpp
// Author      : Martin
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "Opcodes.hpp"
#include <unistd.h>
using namespace std;

int main(int argc, int argv[]) {
	std::cout << "Started";
    Opcodes chip;
    chip.loadGame();
    chip.initialize();



	while(true)
	{
		chip.emulateCycle();
		usleep(100000);
		chip.draw();

	}
	return 0;
}


