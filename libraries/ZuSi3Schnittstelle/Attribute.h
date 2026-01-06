/*
* Author:	Sebastian Wolf
* Created:	August 2018
*/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H
#include <Arduino.h>

class Attribute {
public:
	Attribute(uint16_t ID);
	Attribute(uint16_t ID, byte *DATA, int sizeData);
	Attribute(uint16_t ID, int DATA);
	Attribute(uint16_t ID, float DATA);
	Attribute(uint16_t ID, String DATA);
	Attribute(byte *ID, byte *DATA, int sizeData);
	~Attribute();

	byte *get();
	int getSize();
	int getDATASize();

	byte *getID();
	int getIDAsInt();
	byte *getDATA();
	String getDATAAsString();
	int getDATAAsInt();
	float getDATAAsFloat();
	boolean getDATAAsBoolean();

private:
	byte * ID;
	byte *DATA;

	int sizeID = 2;
	int sizeDATA = -1;
	int PACKET_SIZE;
};
#endif
