#pragma once
#include <Arduino_JSON.h>
#define DEBUG
//#define TRACE

class debug {
public:
 
static void print(String text){
	#ifdef DEBUG 
	Serial.print(text);
	#endif
	}
static void println(String text){
	#ifdef DEBUG 
	Serial.println(text);
	#endif
	}
static void print(int zahl){
	#ifdef DEBUG 
	Serial.print(zahl);
	#endif
	}
static void println(int zahl){
	#ifdef DEBUG 
	Serial.println(zahl);
	#endif
	}
static void printlnJV(JSONVar jsonvar){
	#ifdef DEBUG 
	Serial.println(jsonvar);
	#endif
	}
};

class trace {
public:
 
static void print(String text){
	#ifdef TRACE 
	Serial.print(text);
	#endif
	}
static void println(String text){
	#ifdef TRACE 
	Serial.println(text);
	#endif
	}
static void print(int zahl){
	#ifdef TRACE 
	Serial.print(zahl);
	#endif
	}
static void println(int zahl){
	#ifdef TRACE 
	Serial.println(zahl);
	#endif
	}
static void printlnJV(JSONVar jsonvar){
	#ifdef TRACE 
	Serial.println(jsonvar);
	#endif
	}
};
