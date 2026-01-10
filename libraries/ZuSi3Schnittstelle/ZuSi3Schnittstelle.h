/*
* Author:	Sebastian Wolf
* Created:	August 2018
*/

#ifndef ZUSI3SCHNITTSTELLE_H
#define ZUSI3SCHNITTSTELLE_H

#include <NetworkClient.h>
#include <LinkedList.h>
#include "Node.h"
#include "Fuehrerstandsanzeigen.h"
#include "ID_Definition.h"

class Zusi3Schnittstelle {
public:
	Zusi3Schnittstelle(NetworkClient *client, String clientName);
	~Zusi3Schnittstelle();
	boolean connect();
	void close();
	void requestFuehrerstandsanzeigen(int request);
	void reqFstAnz(int request);
	void requestFuehrerstandsbedienung(boolean value);
	void requestProgrammdaten(boolean value);
	void requestProgrammdatenOhneFahrplan(boolean value);
	void setDebugOutput(boolean output);
	void inputSchalterposition(uint16_t zuordnung, int16_t position);
	void inputTastatureingabe(uint16_t zuordnung, uint16_t kommando, uint16_t aktion, int16_t position, float parameter=0);
	void sendTastatureingaben();
	String getZusiVersion();
	String getVersion;
	String getVerbindungsinfo();
	Node *update();

private:
	void HELLO();
	void ACK_HELLO();
	void NEEDED_DATA();
	void ACK_NEEDED_DATA();
	void checkConnection();

	Node *getNodes(byte *rootID);
	boolean compare(byte *a, byte *b, int size);

	byte *HEADER = new byte[4]{ 0x00, 0x00, 0x00, 0x00 };
	byte *ENDE = new byte[4]{ 0xFF, 0xFF, 0xFF, 0xFF };

	NetworkClient *client;
	LinkedList<int> *requestListFuehrerstandsanzeigen = new LinkedList<int>();
	boolean reqFuehrerstandsbedienung = false;
	boolean reqProgrammdaten = false;
	boolean reqProgrammdatenOhneFahrplan = false;

	Node *node = NULL;
	Node *Tastatureingaben = NULL;

	String versionZusi = "unknown";
	String verbindungsinfoZusi = "unknown";

	String clientName;
	String ip;
	int port;
	String version = "0.8.1 beta";
	
	boolean debugOutput = false;
	int reconnectCounter = 1;

};
#endif
