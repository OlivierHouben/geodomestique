/*
Name:		MAIL_V1.ino
Created:	27/03/2018 15:27:42
Author:	Florian, Olivier
*/

/* Pin used
7	RX
8	TX
13	led

A1	Current1
A2	Current2
A3	Current3
A4	Voltage
*/

// Select your modem:
#define TINY_GSM_MODEM_SIM800
//#define TINY_GSM_MODEM_SIM900
//#define TINY_GSM_MODEM_A6
//#define TINY_GSM_MODEM_A7
//#define TINY_GSM_MODEM_M590

// uncomment line for debug
#define _DEBUG_

// Can be installed from Library Manager or https://github.com/vshymanskyy/TinyGSM
#include <TinyGsmClient.h>
#include <ThingerTinyGSM.h>

// Emulate Serial1 on pins 10/11 if HW is not present (use interrupt pin in RX for better performance)
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
// RX, TX
SoftwareSerial Serial1(7, 8);
#endif

// Include Emon Library
#include "EmonLib.h"


// Define device parameters

// Olivier
/*
// Define device
#define USERNAME "houbeno"
#define DEVICE_ID "arduino"
#define DEVICE_CREDENTIAL "sisisi"

// Define APN config
#define APN_NAME "internet.proximus.be"
#define APN_USER ""
#define APN_PSWD ""

// Define cad pin (optional)
#define CARD_PIN ""
*/

// Florian
// /*
// Define device
#define USERNAME "GeoelecTest052"
#define DEVICE_ID "arduino_baw_0001"
#define DEVICE_CREDENTIAL "sisisi"

// Define APN config
#define APN_NAME "gprs.base.be"
#define APN_USER "base"
#define APN_PSWD "base"

// Define cad pin (optional)
#define CARD_PIN ""
// */

// Define id compteur
#define Idcompteur 001
#define IdDoor 001

// Create instance EnergyMonitor
EnergyMonitor emon1;
EnergyMonitor emon2;
EnergyMonitor emon3;

// Current Pins
int emoncurrentPins[3] = { A1,A2,A3 };

// Voltage Pins
// int emonvoltagepins[3]= { a4,a5,a0};

// Real power
float realpower[3] = { 0,0,0 };

// Current, voltage, kW
volatile float Irmstableau[4] = { 0, 0,0,0 };
volatile float Vrmstableau[3] = { 0,0,0 };
volatile float kilos[3] = { 0,0,0 };
volatile float kilosTot = 0;

// Current
float Irms1;
float Irms2;
float Irms3;

// Time
unsigned long startMillis;
unsigned long endMillis = 0;

unsigned long previoustime = 0;
unsigned long currenttime = 0;

unsigned int interval = 30000;

// Initialize interupt
int positionOFF = -1;
int positionON = -1;

// Relay connected to pin 13
const int relay = 13;

// Configure software serial port
// Attention 2 fois les mêmes pins pour le software serial SIM900 et le Serial1
//SoftwareSerial SIM900(7, 8);


ThingerTinyGSM thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL, Serial1);

int speed1;

bool emailsent;

int tp_2;

void setup() {
	// Initializing serial commmunication
	// Uncomment line for debug
	Serial.begin(9600);
	// SIM900.begin(19200);
	// Serial for AT commands (can be higher with HW Serial, or even lower in SW Serial)
	Serial1.begin(9600);

	// set APN (you can remove user and password from call if your apn does not require them)
	thing.setAPN(APN_NAME, APN_USER, APN_PSWD);

	// set PIN (optional)
	// thing.setPIN(CARD_PIN);

	emailsent = 0;
	speed1 = 0;
	tp_2 = 0;

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


	thing["relay"] << [](pson& in) {
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


	thing["SPEED"] << inputValue(speed1, {});
	//thing["emailsent"] << inputValue(emailsent, {});
	//thing["emailsentout"] >> outputValue(emailsent);
	// resource output example (i.e. reading a sensor value)
	thing["millis"] >> outputValue(millis());
	//thing["tweet_test"] = []() {
		//call_temperature_endpoint();
	//};

	// more details at http://docs.thinger.io/arduino/

}

void loop() {

	// Read the all variables
	readPhase();

	thing.handle();

	if (speed1 > 200 & tp_2 == 0)
	{
		tp_2 = 1;

		pson data_alert;
		data_alert["door_nbr"] = IdDoor;
		data_alert["time"] = millis();

		// Endpoint
		thing.call_endpoint("emailtest", data_alert);

	}

	if (speed1>150 & emailsent == 0)
	{
		//call_temperature_endpoint();
		emailsent = 1;
		// call endpoint

		pson data;
		// Id compteur
		data["ID_Meter"] = Idcompteur;

		// Energy kWh
		data["NRJ1(kwh)"] = kilos[0];
		data["NRJ2(khw)"] = kilos[1];
		data["NRJ3(khw)"] = kilos[2];
		data["NRJTot(khw)"] = kilos[0] + kilos[1] + kilos[2];

		// Current
		data["Irms1(A)"] = Irmstableau[0];
		data["Irms2(A)"] = Irmstableau[1];
		data["Irms3(A)"] = Irmstableau[2];

		// Voltage
		data["Vrms(V)"] = Vrmstableau[0];

		// Data Bucket
		// For using the write_bucket call over a bucket, it is necessary to set the bucket source to "From Write Call"
		thing.write_bucket("BucketBaw001", data);

		// Pas bien compris son fonctionnement
		// ESP.deepSleep(SLEEP_MS*1000, WAKE_RF_DEFAULT); 

		// Endpoint
		// thing.call_endpoint("emailtest", data);
	}


}

void call_temperature_endpoint()
{
	digitalWrite(LED_BUILTIN, LOW);
	pson tweet_content;

	tweet_content["value1"] = Idcompteur;
	tweet_content["value2"] = millis();

	thing.call_endpoint("emailtest", tweet_content);
	digitalWrite(LED_BUILTIN, HIGH);

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
		// Calculate kilowatt hours used
		// Attention peut être en Wh et pas en kWh
		kilos[i] = kilos[i] + (realpower[i] * (time / 3600 / 1000));
	}

	startMillis = millis();

	// Use to debug
	/*
	Serial.println("");
	Serial.println("valeur dans read phase");
	Serial.print(" ancienne IRMS2  ");
	Serial.println(Irms2);
	Serial.print("emon1 Irms   ");
	Serial.println(emon1.Irms);
	Serial.print("apparent power  emon   ");
	Serial.println(emon1.apparentPower);
	Serial.print("realpower2 ancienne ");
	Serial.println(realpower[1]);
	Serial.print("voltage emon  ");
	Serial.println(Vrmstableau[0]);

	Serial.print("verif real power emon  ");
	Serial.println(Irms1*emon1.Vrms);

	Serial.print("kilos1  ");
	Serial.println(kilos[0]);

	Serial.print("kilos2  ");
	Serial.println(kilos[1]);
	*/
}