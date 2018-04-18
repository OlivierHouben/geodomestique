/*
Name:    CALIB_V1.ino
Created: 05/04/2018 10:42:04
Author:  Florian
Pin used:
A1 Current1
A2 Current2
A3 Current3
A4 Voltage
*/

// uncomment line for debug
//#define _DEBUG_

// Include Emon Library
#include "EmonLib.h"

// Create instance EnergyMonitor
EnergyMonitor emon1;
EnergyMonitor emon2;
EnergyMonitor emon3;

// Define GPIO

// Current Pins
int const emoncurrentPins[3] = { A1,A2,A3 };
// Voltage Pins
// int const emonvoltagepins[3]= {A4,A5,A0};

// Initialize result values

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

// Time
unsigned long startMillis;
unsigned long endMillis = 0;
unsigned long time_now = 0;
int period = 5000;
// the setup function runs once when you press reset or power the board
void setup() {
	// Initializing serial commmunication
	Serial.begin(9600);

	// Voltage: input pin, calibration, phase_shift
	emon1.voltage(A4, 116.26, 1.7);

	// Current: input pin, calibration.
	emon1.current(emoncurrentPins[0], 29.1);
	emon2.current(emoncurrentPins[1], 29.1);
	emon3.current(emoncurrentPins[2], 29.1);

	// startMillis = millis();
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (millis() - time_now > period)
	{
		readPhase();
		printReadPhase();
		time_now = millis();
	}

}

void readPhase()
{
	// Method to read information from CTs
	startMillis = millis();

	// Calculate all. No.of half wavelengths (crossings), time-out
	emon1.calcVI(20, 2000);

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
	// Calculate kilowatt hours used
	// Careful in Wh and not in kWh
	//kilos[i] = kilos[i] + (realpower[i] * (time /3600));

	// More fast to do it in differents lines
	// time / 1000 to get time in s and not in ms
	// time / 3600000 to get time in h and not in ms
	time = time / 1000l;
	//time = time / 3600000l;
	// Nowaday kilos is in Ws and not in kWs or kWh or Wh
	kilos[3] = 0.00f;
	for (int i = 0; i <= 2; i++)
	{
		realpower[i] = realpower[i] * time;
		kilos[i] = kilos[i] + realpower[i];
		// NRJ TOT
		kilos[3] = kilos[3] + kilos[i];
	}
}

void printReadPhase()
{
	Serial.println("Read phase values");
	// Print out all variables (realpower, apparent power, Vrms, Irms, power factor)
	// emon1.serialprint();

	Serial.println("");
	Serial.println("kilos 1: ");
	Serial.println(kilos[0]);
	Serial.println("kilos 2: ");
	Serial.println(kilos[1]);
	Serial.println("kilos 3: ");
	Serial.println(kilos[2]);
	Serial.println("kilos T: ");
	Serial.println(kilos[3]);

	Serial.println("");
	Serial.println("Current 1");
	Serial.println(Irmstableau[0]);
	Serial.println("Current 2");
	Serial.println(Irmstableau[1]);
	Serial.println("Current 3");
	Serial.println(Irmstableau[2]);
	Serial.println("Current 1 v2");
	Serial.println(Irmstableau[3]);

	Serial.println("");
	Serial.println("Volatage 1");
	Serial.println(Vrmstableau[0]);
	Serial.println("Volatage 2");
	Serial.println(Vrmstableau[1]);
	Serial.println("Volatage 3");
	Serial.println(Vrmstableau[2]);

	Serial.println("");
	Serial.println("Realpower 1");
	Serial.println(realpower[0]);
	Serial.println("Realpower 2");
	Serial.println(realpower[1]);
	Serial.println("Realpower 3");
	Serial.println(realpower[2]);

}