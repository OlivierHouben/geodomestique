/*
* Complete Project Details http://randomnerdtutorials.com
*/

// Include Software Serial library to communicate with GSM
#include <SoftwareSerial.h>
#include <Time.h>

#include "EmonLib.h"             // Include Emon Library

#define idmeter 1;


EnergyMonitor emon1;             // Create an instance
EnergyMonitor emon2;             // Create an instance
EnergyMonitor emon3;
int emoncurrentPins[3] = { A1,A2,A3 };
//int emonvoltagepins[3]= { a4,a5,a0};
float realpower[3] = { 0,0,0 };
volatile float Irmstableau[4] = {0, 0,0,0 };
volatile float Vrmstableau[3] = { 0,0,0 };

volatile float kilos[3] = {0,0,0};
unsigned long startMillis;
unsigned long endMillis=0;
float Irms1;
float Irms2;
float Irms3;
// Configure software serial port
SoftwareSerial SIM900(7, 8);

// Variable to store text message
String textMessage;

// Create a variable to store Lamp state
String lampState = "HIGH";

//


// Relay connected to pin 12
const int relay = 13;

unsigned long previoustime = 0;
unsigned long currenttime = 0;

unsigned int interval = 30000;


int positionOFF = -1;
int positionON = -1;

void setup() {
	// Automatically turn on the shield
	
	// Set relay as OUTPUT
	pinMode(relay, OUTPUT);

	// By default the relay is off
	digitalWrite(relay, HIGH);

	// Initializing serial commmunication
	Serial.begin(9600);
	SIM900.begin(19200);

	// Give time to your GSM shield log on to network
	delay(7000);
	Serial.print("SIM900 ready...");

	// AT command to set SIM900 to SMS mode
	SIM900.print("AT+CMGF=1\r");
	delay(100);
	// Set module to send SMS data to serial out upon receipt 
	SIM900.print("AT+CNMI=2,2,0,0,0\r");
	delay(100);

	digitalWrite(13, HIGH);
	delay(500);
	digitalWrite(13, LOW);
	delay(500);

	previoustime = millis();

	emon1.voltage(A4, 116.26, 1.7);  // Voltage: input pin, calibration, phase_shift
	emon1.current(emoncurrentPins[0], 29.1);       // Current: input pin, calibration.
	
	emon2.current(emoncurrentPins[1], 29.1);       // Current: input pin, calibration.
	
	emon3.current(emoncurrentPins[2], 29.1);       // Current: input pin, calibration.


	startMillis = millis();
}

void loop() {

		// reinitialisation de la position du mot clé
	
	
	readPhase();
	delay(50);	

	
	if (SIM900.available()>0) {
		 positionOFF = -1;
		 positionON = -1;
		textMessage = SIM900.readString();
		textMessage.toLowerCase();
		Serial.print(textMessage);
		delay(10);
		
		positionON = textMessage.lastIndexOf("on");
		
		positionOFF = textMessage.lastIndexOf("off");
		
	}

	if (positionON>positionOFF && positionON >= 0) {
		// Turn on relay and save current state
		digitalWrite(relay, HIGH);
		lampState = "on";
		Serial.println("Relay set to ON");
		
		textMessage = "";
	}
	if (positionOFF>positionON && positionOFF >= 0) {
		// Turn off relay and save current state
		digitalWrite(relay, LOW);
		lampState = "off";
		Serial.println("Relay set to OFF");
		
		textMessage = "";
	}
	if (millis() - previoustime > interval) // condition d'envoi automatique si le délai a été dépassé
	{
		previoustime = millis();
		Sendinfodata();		
		textMessage = "";


	}
	if (textMessage.indexOf("state") >= 0) { // conditions d'envoi lors de la reception d'un message STATE
		
		
		previoustime = millis();
		Sendinfodata();		
		textMessage = "";
	}
}

void Sendinfodata() 
{
	String message;
	int IDMETER = idmeter;
	// partie ID
	message.concat("IDmeter");
	message.concat(";");
	message.concat(IDMETER);
	message.concat(";");
	// partie kilos
	message.concat("NRJ1(kwh)");
	message.concat(";");
	message.concat(kilos[0]);
	message.concat(";");
	message.concat("NRJ2(kwh)");
	message.concat(";");
	message.concat(kilos[1]);
	message.concat(";");
	message.concat("NRJ3(kwh)");
	message.concat(";");
	message.concat(kilos[2]);
	message.concat(";");

	// partie courant instant et voltage instant
	message.concat("Irms1(A)");
	message.concat(";");
	message.concat(Irmstableau[0]);
	message.concat(";");
	message.concat("Irms2(A)");
	message.concat(";");
	message.concat(Irmstableau[1]);
	message.concat(";");
	message.concat("Irms3(A)");
	message.concat(";");
	message.concat(Irmstableau[2]);
	message.concat(";");	
	message.concat("Vrms(V)");
	message.concat(";");
	message.concat(Vrmstableau[0]);
	message.concat(";");



	Serial.println("");
	Serial.println("Message sms");
	Serial.println(message);
	Serial.println("valeur input");
	Serial.println(analogRead(emoncurrentPins[0]));
	

	sendSMS(message);
	Serial.println("Meter state resquest");

}

void readPhase()      //Method to read information from CTs
{
	emon1.calcVI(20, 2000);         // Calculate all. No.of half wavelengths (crossings), time-out
		
									
									//emon1.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)
	Irms2 = emon2.calcIrms(1480);
	Irms3 = emon3.calcIrms(1480);         // 
	Irms1 = emon1.calcIrms(1480);
	 realpower[0] = emon1.realPower;
	 realpower[1] = Irms2*emon1.Vrms;
	 realpower[2] = Irms3*emon1.Vrms;

	 Irmstableau[0] = emon1.Irms;
	 Irmstableau[1] = Irms2;
	 Irmstableau[2] = Irms3;
	 Irmstableau[3] = Irms1;

	 Vrmstableau[0] = emon1.Vrms;// approximation du meme voltage RmS sur les trois phases
	 Vrmstableau[1] = emon1.Vrms;
	 Vrmstableau[2] = emon1.Vrms;

	endMillis = millis();
	unsigned long time = (endMillis - startMillis);	
	for (int i = 0; i <= 2; i++)
	{
		kilos[i] = kilos[i] + (realpower[i] * (time/3600 / 1000));    //Calculate kilowatt hours used
		
	}
	
	startMillis = millis();
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




	
}




// Function that sends SMS
void sendSMS(String message) {
	// AT command to set SIM900 to SMS mode
	SIM900.print("AT+CMGF=1\r");
	delay(100);

	// REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
	// USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
	SIM900.println("AT + CMGS = \"0494783233\"");
	delay(100);
	// Send the SMS
	SIM900.println(message);
	delay(100);

	// End AT command with a ^Z, ASCII code 26
	SIM900.println((char)26);
	delay(100);
	SIM900.println();
	// Give module time to send SMS
	delay(5000);
}