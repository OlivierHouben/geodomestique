/*
Name:		SMS_V3.ino
Created:	31/03/2018 11:15:18
Author:	Florian
Pin used:
7	RX
8	TX
13	led

A1	Current1
A2	Current2
A3	Current3
A4	Voltage
*/

// Define the modem
#define TINY_GSM_MODEM_SIM800
#define SerialMon Serial
#define SerialAT Serial1

// Include tiny et thinger
#include <TinyGsmClient.h>

// Emulate Serial1 on pins 10/11 (7,8) if HW is not present (use interrupt pin in RX for better performance)
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
// RX, TX
//SoftwareSerial Serial1(8, 7);
SoftwareSerial SerialAT(8, 7);
#endif

String text_sms;

// uncomment line for debug
//#define _DEBUG_

// Define APN config thinger
#define APN_NAME "gprs.base.be"
#define APN_USER "base"
#define APN_PSWD "base"

// Define APN config tiny
const char apn[] = "gprs.base.be";
const char user[] = "base";
const char pass[] = "base";

// Define cad pin (optional)
#define CARD_PIN ""

// Define id meter
#define Idcompteur 001

// Define id door
//#define IdDoor 001

// Relay connected to pin 13
const int relay = 13;

//Time
unsigned long startMillis;
unsigned long endMillis = 0;

unsigned long previoustime = 0;
unsigned long currenttime = 0;

unsigned int interval = 30000;

// Initialize interupt
int positionOFF = -1;
int positionON = -1;

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

// the setup function runs once when you press reset or power the board
void setup() {

	// Initializing serial commmunication with the Serial Monitor
	SerialMon.begin(9600);
	delay(10);

	// Initializing Serial for AT commands
	SerialAT.begin(9600);
	delay(3000);

	// Initializing modem
	SerialMon.println("Initializing modem...");
	modem.restart();

	// Modem information
	String modemInfo = modem.getModemInfo();
	SerialMon.print("Modem: ");
	SerialMon.println(modemInfo);

	// IMEI infomation
	String imei = modem.getIMEI();
	SerialMon.print("imei: ");
	SerialMon.println(imei);
	delay(100);

	// AT command to set module to SMS mode
	SerialAT.print("AT+CMGF=1\r");
	delay(100);
	// AT command to set module to send SMS data to serial out upon receipt 
	SerialAT.print("AT+CNMI=2,1,0,0,0\r");
	delay(100);

	// Send sms (number, texte)
	//modem.sendSMS("+32494181048", "Coucou petite peruche");
	//modem.sendSMS("+32494783233", "Coucou petite peruche");

	// set PIN (optional)
	// thing.setPIN(CARD_PIN);

	previoustime = millis();
	startMillis = millis();

	// Set relay as OUTPUT
	pinMode(relay, OUTPUT);

	// By default the relay is off
	// digitalWrite(relay, HIGH);

}

// the loop function runs over and over again until power down or reset
void loop() {
	if (SerialAT.available() > 0)
	{
		//String buffer;
		//while (SerialAT.available())
		//{
			//char c = SerialAT.read();
			//buffer.concat(c);
			//delay(10);

			text_sms = SerialAT.readString();
			SerialMon.print(text_sms);
			delay(10);
		//}
		//SerialMon.print(buffer);
		//text_sms = SerialAT.readString();
		//SerialMon.print(text_sms);
		//delay(10);

	}
	//SerialMon.println("wait start");
	//delay(30000);
	//SerialMon.println("wait stop");

}
