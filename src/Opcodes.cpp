/*
 * Opcodes.cpp
 *
 *  Created on: 22 Jan 2016
 *      Author: martin
 */

#include "Opcodes.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

int skipNextInstruction=4;
bool clearScreen =false;
int carryRegister =16;
int numberOfOpcodesRun =0;

unsigned short stack[16]={0}; //16 levels of the stack.
unsigned short sp;    //stack pointer to know where the stack is.
unsigned char V[16]={0}; //1 to 15 is general 8 it registers, sixteen register is use for carry flag
//0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
//0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
//0x200-0xFFF - Program ROM and work RAM
unsigned short I;  //index register
unsigned short pc; //program counter. fetches the next address from the application.

unsigned short opcode;  //2 bytes to store the opcode
unsigned char memory[4096];  //4k of memory
//const unsigned short BUFFERSIZE = 3583;
const unsigned short BUFFERSIZE = 550;
unsigned char buffer[BUFFERSIZE];
unsigned char gfx[64 * 32]={0}; //2048 pixels 64x32

unsigned char delay_timer; //60hz and counts down when above zero
unsigned char sound_timer; //60hz and counts down when above zero

unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Opcodes::drawPicture() {
	for (int index = 0; index < 2048; index++) {
		unsigned char pixel = gfx[index];
		if (index % 64 == 0) {
			std::cout << " " << std::endl;
		}
		if (pixel == 0xff) {
			std::cout << "x";
		} else {
			std::cout << "0";
		}
	}
	std::cout << " " << std::endl;
}

void Opcodes::decode(unsigned short opcode){

	unsigned char nn;
	unsigned short nnn;
	unsigned char regX;
	unsigned char regY;
	std::cout << "Operation code : 0x" << std::hex << opcode << std::endl;

	switch(opcode & 0xF000)
	{
	case 0x0000:
		switch(opcode & 0x00FF)
		{
		case 0x00E0:
			//00E0 	Clears the screen.
			clearScreen = true;
			std::cout << "==== clear the screen." << std::endl;
			pc += 2;
			break;
		case 0x00EE:
			//00EE 	Returns from a subroutine.
			std::cout << "==== REturn from subroutine" << std::endl;
			pc = stack[sp];
			sp--;
			pc += 2;
			break;
		default:
			std::cout << "==== Unknown opcode \n";
			break;
		}
		break;
	case 0x1000:
	{
		//Jumps to address NNN.
		//Do not have to store stackpointer here. This is for for loops
		unsigned short address = opcode  & 0x0FFF;
		/*stack[sp]=pc;
		sp++;*/
		pc=address;
		std::cout << "==== Jump to address Program counter: 0x"  << std::hex << pc <<std::endl;
		break;
	}
	case 0x2000:
	{
		//Calls subroutine at NNN.
		unsigned short subroutine = opcode & 0x0FFF;
		std::cout << "==== Calling sub routine" << std::endl;
		//sp++if(sp!=0)
		//{
			sp++;
		//}
		stack[sp]=pc;
		pc=subroutine;
		break;
	}
	case 0x3000:
	{
		//3XNN 	Skips the next instruction if VX equals NN.
		nn = opcode & 0x00FF;
		regX = (opcode & 0x0F00)>>8;
		if(V[regX] == nn)
		{
			std::cout << "Skip next instruction. Equal" << std::endl;
			pc+=4;
		}
		else
		{
			pc += 2;
			std::cout << "===== Not Skipping next instruction. Not Equal regX "<< V[regX] << " nn " << nn << std::endl;
		}
		break;
	}
	case 0x4000:
	{
		//4XNN 	Skips the next instruction if VX doesn't equal NN.
		nn = opcode & 0x00FF;
		regX = (opcode & 0x0F00)>>8;
		if(V[regX] != nn)
		{
			std::cout << "Skip next instruction. Not Equal" << std::endl;
			pc+=4;
		}
		else
		{
			pc += 2;
			std::cout << "===== Not Skipping next instruction. Equal regX "<< V[regX] << " nn " << nn << std::endl;
		}
		break;
	}
	case 0x5000:
		//5XY0 	Skips the next instruction if VX equals VY.
		regX = (opcode & 0x0F00)>>8;
		regY = (opcode & 0x00F0)>>4;
		if(V[regX] == V[regY])
		{
			std::cout << "Skip next instruction. Equal" << std::endl;
			pc+=4;
		}
		else
		{
			pc += 2;
			std::cout << "===== Not Skipping next instruction. Equal regX "<< V[regX] << " regY " << V[regY] << std::endl;

		}
		break;
	case 0x6000:
		//6XNN 	Sets VX to NN.
		nn = (opcode & 0x00FF);
		regX = (opcode & 0x0F00)>>8;
		V[regX] =nn;
		std::cout << "===== Set RegX to NN regx"<< regX <<" regX "<< V[regX] << " nn " << nn << std::endl;
		pc += 2;
		break;

	case 0x7000:
		//7XNN 	Adds NN to VX.
		nn = opcode & 0x00FF;
		regX = (opcode & 0x0F00)>>8;
		V[regX] = V[regX] + nn;
		std::cout << "===== Add NN to RegX regx"<< regX <<" regX "<< V[regX] << " nn " << nn << std::endl;
		pc += 2;
		break;
	case 0x8000:
	{
		regX = (opcode & 0x0F00)>>8;
		regY = (opcode & 0x00F0)>>4;
		switch(opcode & 0x000F)
		{
			case 0x0000:
				//8XY0 	Sets VX to the value of VY.
				V[regX]=V[regY];
				pc += 2;
				break;
		    case 0x0001:
		    	//8XY1 	Sets VX to VX or VY.
		    	V[regX] = regX | regY;
		    	pc += 2;
		    	break;
		    case 0x0002:
		    	//8XY2 	Sets VX to VX and VY.
		    	V[regX]=regX & regY;
		    	pc += 2;
		    	break;
		    case 0x0003:
		    	//8XY3 	Sets VX to VX xor VY.
		    	V[regX]= regX xor regY;
		    	pc += 2;
		    	break;
		    case 0x0004:
		    	//8XY4 	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
		    	if(V[regX]+V[regY] > 255)
		    	{
		    		V[carryRegister] = 1;
		    	}
		    	else
		    	{
		    		V[carryRegister] = 0;
		    	}
		    	V[regX] = V[regX]+V[regY];
		    	pc+=2;
		    	break;

		    case 0x0005:
		    	//8XY5 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
				if (V[regX] - V[regY] < 255)
				{
					V[carryRegister] = 1;
				} else {
					V[carryRegister] = 0;
				}
				V[regX] = V[regX] - V[regY];
				pc += 2;
				break;

		    case 0x0006:
		    	//8XY6 	Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
		    	V[carryRegister] = V[regX] && 0x0001;
		    	V[regX] = V[regX] >> 1;
		    	pc += 2;
		    	break;
		    case 0x0007:
		    	//8XY7 	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
				if (V[regX] - V[regY] < 255)
				{
					V[carryRegister] = 1;
				} else {
					V[carryRegister] = 0;
				}
				V[regX] = V[regY] - V[regX];
				pc += 2;
				break;

		    case 0x000E:
		    	//8XYE 	Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
		    	V[carryRegister] = V[regX] & 0x8000;
		    	V[regX] = V[regX] << 1;
		    	pc += 2;
		    	break;

		}
		break;
	}
	case 0x9000:
	{
		//9XY0 	Skips the next instruction if VX doesn't equal VY.
		regX = (opcode & 0x0F00)>>8;
		regY = (opcode & 0x00F0)>>4;
		if(regX == regY)
		{
			std::cout << "Skip next instruction " << std::endl;
			pc+=skipNextInstruction;
		}
		else
		{
			pc += 2;
		}
		break;
	}
	case 0xA000:
	{
		//ANNN 	Sets I to the address NNN.
		std::cout << "==== Setting index register" << std::endl;
		nnn = opcode & 0x0FFF;
		I = nnn;
		pc += 2;
		break;
	}
	case 0xB000:
	{
		//BNNN 	Jumps to the address NNN plus V0.
		//Do not have to store the stackpointer here. This jump is for for loops
		nnn = opcode & 0x0FFF;
		/*stack[sp]=pc;
		sp++;*/
		pc=V[0] + nnn;
		break;
	}
    case 0xC000:
    {
    	//CXNN 	Sets VX to the result of a bitwise and operation on a random number and NN.
    	nn = opcode & 0x00FF;
    	regX = (opcode & 0x0F00)>>8;
    	V[regX]=nn & 1;
    	pc += 2;
    	break;
    }
    case 0xD000:
    {
    	regX = (opcode & 0x0F00) >>8;
    	regY = (opcode & 0x00F0) >>4;
    	char N = (opcode & 0x000F);
    	int xInt = regX;
    	int nextLineUp=64;
    	//TODO
    	/*
    	 * DXYN 	Sprites stored in memory at location in index register (I), 8bits wide. Wraps around the screen.
    	 * If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen pixels).
    	 * Sprites are drawn starting at position VX, VY. N is the number of 8bit rows that need to be drawn.
    	 * If N is greater than 1, second line continues at position VX, VY+1, and so on.
    	 */

    	for(int y=regY;y<=N;y++)
    	{
    		for(int x=xInt;x<xInt+8;x++)
    		{
    			gfx[x]=0xFF;
    		}
    		xInt+=nextLineUp;
    	}
		drawPicture();
    	pc += 2;
    	break;
    }
	case 0xE000:
	{
		switch(opcode & 0x00FF)
		{
			case 0x009E:
			{
				std::cout << "Have not done this yet !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Skips the next instruction if the key stored" << std::endl;
				//EX9E 	Skips the next instruction if the key stored in VX is pressed.
				unsigned char reg = (opcode & 0x0F00)>>8;
				//TODO
				if(V[reg])
				{
					std::cout << "Skip next instruction" <<std::endl;
					pc+=2;
				}
				else
				{
					pc += 2;
				}
				break;
			}
		    case 0x00A1:
		    {
		    	std::cout << "Have not done this yet !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Skips the next instruction if the key stored" << std::endl;
		    	//EXA1 	Skips the next instruction if the key stored in VX isn't pressed.
		    	regX = (opcode & 0x0F00)>>8;
		    	//TODO
		    	if(V[regX])
		    	{
		    		std::cout << "Skip next instruction" << std::endl;
		    		pc+=2;
		    	}
		    	else
		    	{
		    		pc += 2;
		    	}
		    	break;
		    }
		}
		break;
	}
    case 0xF000:
    {
    	switch(opcode & 0x00FF)
    	{
    	case 0x0007:
    		//FX07 	Sets VX to the value of the delay timer.
    		regX = (opcode & 0x0F00)>>8;
    		V[regX]=delay_timer;
    		pc += 2;
    		break;
    	case 0x000A:
    		//FX0A 	A key press is awaited, and then stored in VX.
    		//TODO
    		regX = (opcode & 0x0F00)>>8;
    		/*if()
    		{
    			V[regX];
    		}*/
    		pc += 2;
    		break;
    	case 0x0015:
    		//FX15 	Sets the delay timer to VX.
    		std::cout << "==== Set the delay timer" << std::endl;
    		regX = (opcode & 0x0F00)>>8;
    		delay_timer = V[regX];
    		pc += 2;
    		break;
    	case 0x0018:
    		//FX18 	Sets the sound timer to VX.
    		std::cout << "==== Set the sound timer" << std::endl;
    		regX = (opcode & 0x0F00)>>8;
    		sound_timer = V[regX];
    		pc += 2;
    		break;
    	case 0x001E:
    		//FX1E 	Adds VX to I.
    		regX = (opcode & 0x0F00)>>8;
    		I = V[regX] +I;
    		pc += 2;
    		break;
    	case 0x0029:
    		//FX29 	Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
    		std::cout << "Setting I to the location of sprite for character in VX : " << V[regX] << std::endl;
    		regX = (opcode & 0x0F00)>>8;
    		I=V[regX]*5;
    		pc += 2;
    		break;
    	case 0x0033:
    	{
    		unsigned char ones = regX % 10;
    		unsigned char tens = regX /10 % 10;
    		unsigned char hundreds = regX /100 % 10;
    		std::cout << "Storing VX binary in to memory at I hundreds:" << hundreds << " Tens:" << tens << " Ones:" << ones << std::endl;
    		memory[I] = hundreds;
    		memory[I+1] = tens;
    		memory[I+2] = ones;
    		//FX33 	Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2.
    		//(In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
    		pc += 2;
    		break;
    	}
    	case 0x0055:
    		//FX55 	Stores V0 to VX in memory starting at address I
    		std::cout << "Storing V0 To VX in memory starting at address I" << std::endl;
    		regX = (opcode & 0x0F00)>>8;
    		for(char index=0;index<regX;index++)
    		{
    			memory[I+index] =V[index];
    		}
    		pc += 2;
    		break;
    	case 0x0065:
    		//FX65 	Fills V0 to VX with values from memory starting at address I.
    		std::cout << "Fill V0 To VX with values from memory starting at address I" << std::endl;
    		regX = (opcode & 0x0F00)>>8;
			for (char index = 0; index < regX; index++) {
				V[index] = memory[I + index];
			}
			pc += 2;
    		break;
    	default:
    	    std::cout << "Unknown OpCode for F case" << std::endl;
    	    break;
    	}
    }
    	default:
    		break;
	}
}

void Opcodes::loadGame()
{
	FILE * file;
	file = fopen("TICTAC","r");

	for(int index =0;index<=BUFFERSIZE;index++)
	{
		memory[index + 512] = 0;
	}

	fread(buffer, sizeof(unsigned char), BUFFERSIZE, file);
	for(int index =0;index<=BUFFERSIZE;index++)
	{
		memory[index + 512] = buffer[index];
		std::cout  << "Opcode read: " << std::hex << buffer[index] <<  std::endl;
	}
	std::cout << "Finished reading game in to memory" << std::endl;
	struct timespec timeOut,remains;

	timeOut.tv_sec = 5;
	timeOut.tv_nsec = 0;

	nanosleep(&timeOut, &remains);

}

void Opcodes::initialize()
{
  pc     = 0x200;  // Program counter starts at 0x200
  opcode = 0;      // Reset current opcode
  I      = 0;      // Reset index register
  sp     = 0;      // Reset stack pointer

  // Clear display
  // Clear stack
  // Clear registers V0-VF
  // Clear memory

  // Load fontset
  for(int i = 0; i < 80; ++i)
    memory[i] = chip8_fontset[i];

  // Reset timers
}

void Opcodes::emulateCycle()
{
	opcode = memory[pc] << 8 | memory[pc + 1];
	std::cout << "Program counter: 0x"  << std::hex << pc <<std::endl;
	std::cout << "Stack Pointer: " << sp <<std::endl;
	numberOfOpcodesRun++;
	std::cout << "Number of opcodes run: " << numberOfOpcodesRun <<std::endl;
	if(sp>16)
	{
		std::cout << "Stack Pointer over flow" <<std::endl;
		exit(EXIT_FAILURE);
	}

	decode(opcode);
}

void Opcodes::draw()
{
	for(int index=0;index<10;index++)
	{
		if(gfx[index]==1)
		{
			//black pixel
		}
		else
		{
			//white pixel
		}

	}
}
