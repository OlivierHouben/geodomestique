/*
Name:		IOT_V1.ino
Created:	05/04/2018 17:29:32
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
#include <ThingerTinyGSM.h>

// Emulate Serial1 on pins 10/11 (7,8) if HW is not present (use interrupt pin in RX for better performance)
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
// RX, TX
SoftwareSerial SerialAT(8, 7);
#endif

// Include Emon Library
#include "EmonLib.h"

// uncomment line for debug
//#define _DEBUG_

// Define device
#define USERNAME "GeoelecTest052"		//thinger username
#define DEVICE_ID "arduino_baw_0001"	// thinger device name
#define DEVICE_CREDENTIAL "sisisi"		// thinger credential

// Define APN config
#define APN_NAME "gprs.base.be"
#define APN_USER "base"
#define APN_PSWD "base"

// Define cad pin (optional)
#define CARD_PIN ""

// Define id meter
#define Idcompteur 001

// Define id door
//#define IdDoor 001

// Create instance EnergyMonitor
EnergyMonitor emon1;
EnergyMonitor emon2;
EnergyMonitor emon3;

// Relay connected to pin 13
const int relay = 13;

// Current Pins
int const emoncurrentPins[3] = { A1,A2,A3 };

// Voltage Pins
// int const emonvoltagepins[3]= {A4,A5,A0};

// Real power
float realpower[3] = { 0.00f,0.00f,0.00f };
// Current, voltage, kW
float Irmstableau[4] = { 0.00f,0.00f,0.00f,0.00f };
float Vrmstableau[3] = { 0.00f,0.00f,0.00f };
float kilos[4] = { 0.00f,0.00f,0.00f,0.00f };

// Current
float Irms1;
float Irms2;
float Irms3;

//Time
unsigned long startMillis;
unsigned long endMillis = 0;

unsigned long previoustime = 0;
unsigned long currenttime = 0;

unsigned int interval = 30000;

// Initialize interupt
int positionOFF = -1;
int positionON = -1;

String text_sms;

int volatile voltage1;
int volatile sendemail1;
bool volatile emailsent;

//TinyGsm modem(SerialAT);
//TinyGsmClient client(modem);
ThingerTinyGSM thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL, SerialAT);

// the setup function runs once when you press reset or power the board
void setup() {

	// Initializing serial commmunication with the Serial Monitor
	SerialMon.begin(9600);
	delay(10);

	// Initializing Serial for AT commands
	SerialAT.begin(9600);
	//delay(3000);

	thing.setAPN(APN_NAME, APN_USER, APN_PSWD);

	// Initializing modem
	//SerialMon.println("Initializing modem...");
	//modem.restart();

	// Modem information
	//String modemInfo = modem.getModemInfo();
	//SerialMon.print("Modem: ");
	//SerialMon.println(modemInfo);

	// IMEI infomation
	//String imei = modem.getIMEI();
	//SerialMon.print("imei: ");
	//SerialMon.println(imei);
	//delay(100);

	// AT command to set module to SMS mode
	//SerialAT.print("AT+CMGF=1\r");
	//delay(100);

	// AT command to set module to send SMS data to serial out upon receipt 
	//SerialAT.print("AT+CNMI=2,2,0,0,0\r");
	//delay(100);

	// set APN (you can remove user and password from call if your apn does not require them)
	

	// set PIN (optional)
	// thing.setPIN(CARD_PIN);

	emailsent = 0;
	voltage1 = 0;
	sendemail1 = 0;
	//tp_2 = 0;

	previoustime = millis();

	// Voltage: input pin, calibration, phase_shift
	emon1.voltage(A4, 116.26, 1.7);

	// Current: input pin, calibration.
	emon1.current(emoncurrentPins[0], 29.1);
	emon2.current(emoncurrentPins[1], 29.1);
	emon3.current(emoncurrentPins[2], 29.1);

	startMillis = millis();

	// resource input example (i.e, controlling a digitalPin);
	pinMode(LED_BUILTIN, OUTPUT);

	// Set relay as OUTPUT
	pinMode(relay, OUTPUT);

	// By default the relay is off
	// digitalWrite(relay, HIGH);


	thing["Relay"] << [](pson& in) {
		if (in.is_empty())
		{
			in = (bool)digitalRead(relay);
		}
		else
		{
			digitalWrite(relay, in ? HIGH : LOW);
		}
		//digitalWrite(13, in ? HIGH : LOW);
	};

	thing["voltage"] << inputValue(voltage1, {});

	thing["TotalNRJ"] >> [](pson& out) {
		//out = Idcompteur;
		out["ID_Meter"] = Idcompteur;
		out["NRJ1"] = kilos[0];
		out["NRJ2"] = kilos[1];
		out["NRJ3"] = kilos[2];
		out["NRJT"] = kilos[3];
		out["Irms1"] = Irmstableau[0];
		out["Irms2"] = Irmstableau[1];
		out["Irms3"] = Irmstableau[2];
		out["Vrms"] = Vrmstableau[0];
	};

	// Send sms (number, texte)
	//modem.sendSMS("num", "text");
}

// the loop function runs over and over again until power down or reset
void loop() {
	//readPhase();

	thing.handle();

	/*if (SerialAT.available() > 0)
	{
		text_sms = SerialAT.readString();
		SerialMon.print(text_sms);
		delay(10);
	}*/

	/*if (voltage1 > 100)
	{
		thing.stop();
		SerialAT.print("AT+CMGF=1\r");
		delay(100);

		// AT command to set module to send SMS data to serial out upon receipt 
		SerialAT.print("AT+CNMI=2,2,0,0,0\r");
		delay(100);
		modem.sendSMS("+num", "text");
		thing.handle();
	}*/

	if (voltage1>150 & emailsent == 0)
	{
		//call_temperature_endpoint();
		emailsent = 1;
		// call endpoint

		pson data;
		// Id compteur
		data["ID_Meter"] = Idcompteur;

		// Energy kWh
		data["NRJ1(kwh)"] = kilos[0];
		data["NRJ2(kwh)"] = kilos[1];
		//data["NRJ3(kwh)"] = kilos[2];
		data["NRJTot(kwh)"] = kilos[3];
		//data["NRJTot"] = kilosTot;

		// Current
		data["Irms1(A)"] = Irmstableau[0];
		data["Irms2(A)"] = Irmstableau[1];
		data["Irms3(A)"] = Irmstableau[2];

		// Voltage
		data["Vrms(V)"] = Vrmstableau[0];

		// Data Bucket
		// For using the write_bucket call over a bucket, it is necessary to set the bucket source to "From Write Call"
		thing.write_bucket("BucketBaw001", data);

		//thing.write_bucket("BucketBaw001", thing["AllData"]);
		//thing.write_bucket("BucketBaw001", "AllData");

		// Pas bien compris son fonctionnement
		// ESP.deepSleep(SLEEP_MS*1000, WAKE_RF_DEFAULT); 

		// Endpoint
		thing.call_endpoint("EndPointBaw001", data);
	}
}

void readPhase()
{
	// Method to read information from CTs

	// Calculate all. No.of half wavelengths (crossings), time-out
	emon1.calcVI(20, 2000);

	// emon1.serialprint();           
	// Print out all variables (realpower, apparent power, Vrms, Irms, power factor)

	// Irms
	Irms1 = emon1.calcIrms(1480);
	Irms2 = emon2.calcIrms(1480);
	Irms3 = emon3.calcIrms(1480);

	// Real power
	realpower[0] = emon1.realPower;
	realpower[1] = Irms2 * emon1.Vrms;
	realpower[2] = Irms3 * emon1.Vrms;

	// Irms
	Irmstableau[0] = emon1.Irms;
	Irmstableau[1] = Irms2;
	Irmstableau[2] = Irms3;
	Irmstableau[3] = Irms1;

	// Vrms
	// Approximation du meme voltage RmS sur les trois phases
	Vrmstableau[0] = emon1.Vrms;
	Vrmstableau[1] = emon1.Vrms;
	Vrmstableau[2] = emon1.Vrms;

	endMillis = millis();
	unsigned long time = (endMillis - startMillis);
	for (int i = 0; i <= 2; i++)
	{
		//Serial.println("");
		//Serial.println("kilos i initiale");
		//Serial.println(kilos[i]);
		// kilos[i] = kilos[i] + 1;
		// Calculate kilowatt hours used
		// Attention peut être en Wh et pas en kWh
		//kilos[i] = kilos[i] + (realpower[i] * (time /3600));

		kilos[i] = kilos[i] + (realpower[i] * (time / 3600 / 1000));

		//Serial.println("");
		//Serial.println("kilos i calc");
		//Serial.println(kilos[i]);

		kilos[3] = kilos[3] + kilos[i];

		//Serial.println("");
		//Serial.println("kilos 3");
		//Serial.println(kilos[3]);
	}

	//kilos[3] = kilos[0];

	startMillis = millis();
}