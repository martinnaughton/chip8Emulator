/*
 * Opcodes.h
 *
 *  Created on: 22 Jan 2016
 *      Author: martin
 */

#ifndef OPCODES_H_
#define OPCODES_H_



class Opcodes {
public:
	void decode(unsigned short opcode);
	void loadGame();
	void initialize();
	void emulateCycle();
	void draw();

private:
	void drawPicture();
};


#endif /* OPCODES_H_ */
