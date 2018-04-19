/*
* Complete Project Details http://randomnerdtutorials.com
*/

// Include Software Serial library to communicate with GSM
#include <Time.h>

#define idmeter 1;

/*#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
// RX, TX
SoftwareSerial SIM900(7, 8);
#endif
*/
// Configure software serial port
#include "SoftwareSerial.h"
// RX, TX
SoftwareSerial SIM900(8, 7);
// Variable to store text message
String textMessage;

// Create a variable to store Lamp state
String lampState = "HIGH";

const int relay = 13;

unsigned long previoustime = 0;

unsigned long interval = 60000;

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
	SIM900.begin(9600);

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
}

void loop() {

	// reinitialisation de la position du mot clé

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
	//Serial.println("Start sms sending");
	String message;
	int IDMETER = idmeter;
	// partie ID
	message.concat("IDmeter");
	message.concat(";");
	message.concat(IDMETER);
	message.concat(";");

	/*
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
	*/
	sendSMS(message);
	Serial.println("Send sms");

}

// Function that sends SMS
void sendSMS(String message) {
	// AT command to set SIM900 to SMS mode
	SIM900.print("AT+CMGF=1\r");
	delay(100);

	// RECIPIENT'S MOBILE NUMBER
	SIM900.println("AT + CMGS = \"num\"");
	//SIM900.println("AT + CMGS = \"num\"");
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