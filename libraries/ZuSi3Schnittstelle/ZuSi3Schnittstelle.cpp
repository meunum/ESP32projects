/*
* Author:	Sebastian Wolf
* Created:	August 2018
*/
#include "Zusi3Schnittstelle.h"

Zusi3Schnittstelle::Zusi3Schnittstelle(NetworkClient *client, String clientName) 
{
	this->client = client;
	this->clientName = clientName;
}

boolean Zusi3Schnittstelle::connect() 
{
	if (client->connected()) 
	{
		HELLO();
		ACK_HELLO();
		NEEDED_DATA();
		ACK_NEEDED_DATA();
		return true;
	}
	return false;
}

Zusi3Schnittstelle::~Zusi3Schnittstelle() 
{
	delete HEADER;
	delete ENDE;
	delete client;
	delete requestListFuehrerstandsanzeigen;
}

void Zusi3Schnittstelle::close() 
{
	client->stop();
}

void Zusi3Schnittstelle::requestFuehrerstandsanzeigen(int request) 
{
	requestListFuehrerstandsanzeigen->add(request);
}

void Zusi3Schnittstelle::reqFstAnz(int request) {
	requestFuehrerstandsanzeigen(request);
}

void Zusi3Schnittstelle::requestFuehrerstandsbedienung(boolean value) 
{
	reqFuehrerstandsbedienung = value;
}

void Zusi3Schnittstelle::requestProgrammdaten(boolean value) 
{
	reqProgrammdaten = value;
}

void Zusi3Schnittstelle::requestProgrammdatenOhneFahrplan(boolean value) 
{
	reqProgrammdatenOhneFahrplan = value;
}

void Zusi3Schnittstelle::setDebugOutput(boolean output) 
{
	debugOutput = output;
}

String Zusi3Schnittstelle::getZusiVersion() 
{
	return versionZusi;
}

String Zusi3Schnittstelle::getVerbindungsinfo() 
{
	return verbindungsinfoZusi;
}

Node *Zusi3Schnittstelle::update() {
	if (node != NULL) 
	{
		delete node;
		node = NULL;
	}
	if (!client->connected()) 
	{
		if (debugOutput) 
		{
			Serial.print("Verbindungsaufbau (");
			Serial.print(reconnectCounter);
			Serial.print(")\n");
		}
		reconnectCounter++;
		if (connect()) 
		{
			reconnectCounter = 1;
		}
	}
	if (client->available() > 19) 
	{
		byte *header = new byte[4];
		client->read(header, 4);
		delete header;

		byte *ID = new byte[2];
		client->read(ID, 2);

		node = getNodes(ID);
		return node;
	}
	return NULL;
}

void Zusi3Schnittstelle::HELLO() 
{
	Node *verbindungsaufbau = new Node(1);
		Node *befehl_HELLO = new Node(1);
			Attribute *protokoll_Version = new Attribute(1, 2);
			Attribute *client_Typs = new Attribute(2, 2);
			Attribute *identifikation = new Attribute(3, clientName);
			Attribute *versionsnummer = new Attribute(4, version);
			befehl_HELLO->addAttribute(protokoll_Version);
		befehl_HELLO->addAttribute(client_Typs);
		befehl_HELLO->addAttribute(identifikation);
		befehl_HELLO->addAttribute(versionsnummer);
	verbindungsaufbau->addNode(befehl_HELLO);
	int length = 0;
	byte *result = verbindungsaufbau->get(&length);
	client->write(result, length);
	delete result;
	//delete verbindungsaufbau; //Absturzgrund bei dem ESP32
}

void Zusi3Schnittstelle::ACK_HELLO() 
{
	while (client->available() < 10) 
	{
		delay(250);
	}
	byte *header = new byte[4];
	client->read(header, 4);
	byte *id = new byte[2];
	client->read(id, 2);
	Node *verbindungsaufbau = getNodes(id);
	if (verbindungsaufbau->getIDAsInt() == 0x01) 
	{
		Node *befehl_ACK_HELLO = verbindungsaufbau->getNodeByID(0x02);
		if (befehl_ACK_HELLO != NULL) {
			int client_Aktzeptiert = befehl_ACK_HELLO->getAttributeByID(3)->getDATAAsInt();
			versionZusi = befehl_ACK_HELLO->getAttributeByID(1)->getDATAAsString();
			verbindungsinfoZusi = befehl_ACK_HELLO->getAttributeByID(2)->getDATAAsString();
			if (debugOutput) {
				Serial.print("client_Aktzeptiert: ");
				Serial.println(client_Aktzeptiert);

				Serial.print("zusi_version: ");
				Serial.println(versionZusi);

				Serial.print("zusi_verbindungsinfo: ");
				Serial.println(verbindungsinfoZusi);
			}
		}
	}
	delete header;
	delete verbindungsaufbau;
}

void Zusi3Schnittstelle::NEEDED_DATA() 
{
	Node *client_Anwendung = new Node(0x02);
		Node *befehl_NEEDED_DATA = new Node(0x03);
			Node *untergruppe_Fuehrerstandsanzeigen = new Node(0x0A);
			for (int i = 0; i < requestListFuehrerstandsanzeigen->size(); i++) {
				Attribute *attr = new Attribute(1, requestListFuehrerstandsanzeigen->get(i));
				untergruppe_Fuehrerstandsanzeigen->addAttribute(attr);
			}
		befehl_NEEDED_DATA->addNode(untergruppe_Fuehrerstandsanzeigen);
		if (reqFuehrerstandsbedienung) {
			Node *untergruppe_Fuehrerstandsbedienung = new Node(0x0B);
			befehl_NEEDED_DATA->addNode(untergruppe_Fuehrerstandsbedienung);
		}
		if (reqProgrammdaten || reqProgrammdatenOhneFahrplan) {
			Node *untergruppe_Programmdaten = new Node(0x0C);
			int max = 4;
			if (reqProgrammdatenOhneFahrplan) {
				max = 3;
			}
			for (int i = 1; i <= max; i++) {
				Attribute *attribute = new Attribute(1, i);
				untergruppe_Programmdaten->addAttribute(attribute);
			}
			befehl_NEEDED_DATA->addNode(untergruppe_Programmdaten);
		}
	client_Anwendung->addNode(befehl_NEEDED_DATA);
	int length = 0;
	byte *result = client_Anwendung->get(&length);
	client->write(result, length);
	delete result;
	delete client_Anwendung;
}

void Zusi3Schnittstelle::ACK_NEEDED_DATA() 
{
	while (client->available() < 10) {
		delay(250);
	}
	byte *header = new byte[4];
	client->read(header, 4);
	byte *id = new byte[2];
	client->read(id, 2);
	Node *Client_Anwendung_02 = getNodes(id);
	if (Client_Anwendung_02->getIDAsInt() == 2) {
		Node *befehl_ACK_NEEDED_DATA = Client_Anwendung_02->getNodeByID(0x04);
		if (befehl_ACK_NEEDED_DATA != NULL) {
			int befehl_Aktzeptiert = befehl_ACK_NEEDED_DATA->getAttributeByID(1)->getDATAAsInt();
			if (befehl_Aktzeptiert == 0) {
				Serial.println("Befehl aktzeptiert");
			}
			else {
				Serial.println("Befehl nicht aktzeptiert");
			}
		}
	}
	delete header;
	delete Client_Anwendung_02;
}

Node *Zusi3Schnittstelle::getNodes(byte *rootID) 
{
	Node *rootNode = new Node(rootID);
	while (client->available() > 0) {
		byte header[4];
		client->read(header, 4);
		if (compare(header, HEADER, 4)) {
			byte *ID = new byte[2];
			client->read(ID, 2);
			Node *subNode = getNodes(ID);
			rootNode->addNode(subNode);
		}
		else if (compare(header, ENDE, 4)) {
			return rootNode;
		}
		else {
			byte *ID = new byte[2];
			client->read(ID, 2);
			int length = ((header[0] & 0xFF) | (header[1] & 0xFF) << 8 | (header[2] & 0xFF) << 16 | (header[3] & 0xFF) << 24);//header[0];
			if (length - 2 > 0) {
				byte *DATA = new byte[length - 2];
				client->read(DATA, length - 2);
				Attribute *attr = new Attribute(ID, DATA, length - 2);
				rootNode->addAttribute(attr);
			}
		}
	}
	return rootNode;
}

void Zusi3Schnittstelle::inputSchalterposition(uint16_t zuordnung, int16_t position)
{
	inputTastatureingabe(zuordnung, 0, 7, position, 0.0);
}

void Zusi3Schnittstelle::inputTastatureingabe(uint16_t zuordnung, uint16_t kommando, uint16_t aktion, int16_t position, float parameter)
{
	if(Tastatureingaben == NULL)
	{
		Tastatureingaben = new Node(ID_Tastatureingaben);
	}
	
	Tastatureingaben->addAttribute(new Attribute(ID_Tastaturzuordnung, zuordnung));
	Tastatureingaben->addAttribute(new Attribute(ID_Tastaturkommando, kommando));
	Tastatureingaben->addAttribute(new Attribute(ID_Schalterposition, position));
	Tastatureingaben->addAttribute(new Attribute(ID_Tastaturaktion, aktion)); 
	Tastatureingaben->addAttribute(new Attribute(ID_Parameter, parameter));
}

void Zusi3Schnittstelle::sendTastatureingaben()
{
	if(Tastatureingaben != NULL)
	{
		Node* clientAnwendung = new Node(ID_ClientAnwendung);
		Node* input = new Node(ID_INPUT);
		clientAnwendung->addNode(input);
		input->addNode(Tastatureingaben);
		int length = 0;
		byte *data = clientAnwendung->get(&length);
		
		client->write(data, length);
		
		delete data;
		delete clientAnwendung;
		Tastatureingaben = NULL;
	}
}

boolean Zusi3Schnittstelle::compare(byte * a, byte * b, int size) 
{
	for (int i = 0; i < size; i++) 
	{
		if (a[i] != b[i]) 
		{
			return false;
		}
	}
	return true;
}
