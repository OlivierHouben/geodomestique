// Serial Relay v2.0 - Arduino >= 1.0 
// EN:
//  *** PATCHED VERSION for Arduino 1.0 ***
//  Arduino will patch serial link between the computer and 
//  the GPRS Shield at 19200 bps 8-N-1
//  Computer is connected to Hardware UART
//  GPRS Shield is connected to the Software UART 
//
//  This version allows to send escape sequence to GPRS Shield (for SMS messaging)
//  use <tilde>z for Ctrl-z
//  use <tilde>e for ESC
//  use <tilde><tilde> for <tilde>
//
//  Require the Arduino serial monitor to be configured on
//  19200 bds + Carriage Return
//
//
// Licence: CC-BY-SA 
// Meurisse D.
// http://www.mchobby.be 
//
#include <SoftwareSerial.h>
// 8 7 pour uno
SoftwareSerial mySerial(11, 10);
//SoftwareSerial mySerial(8, 7);

// EN: 1 means than escape caracter (tilde) has been detected from Serial Monitor (Arduino Side)
// FR: 1 indique que le caractere d'echappement est détecté sur le moniteur serie (coté Arduino)
int escapingFlag = 0;

void setup()
{
	mySerial.begin(9600);               // the GPRS baud rate   
	Serial.begin(9600);                 // the GPRS baud rate   
										//Serial.println("test");
}

void loop()
{
	char SerialInByte;
	if (Serial.available())
	{
		// EN: Listen to Arduino IDE serial Moniteur
		// FR: Ecouter les envois du serial moniteur d'Arduino IDE
		char SerialInByte;
		SerialInByte = Serial.read();

		// EN: Intercept escape character (tilde) for escape sequence
		// FR: Intercepter le caractere d'echappement (tilde) for une sequence d'echappement
		if ((SerialInByte == 126) && (escapingFlag == 0)) {
			escapingFlag = 1;
			return; // wait next caracter / attendre caractere suivant
		}
		else {
			// EN: complete escaping sequence (if appropriate)
			// FR: complete la séquence d'echappement (if appropriate)
			if (escapingFlag == 1) {
				if (SerialInByte == 122) { // (tilde) z = Ctrl Z
					SerialInByte = 26;
				}
				if (SerialInByte == 101) { // (tilde) e = ESC
					SerialInByte = 27;
				}
				escapingFlag = 0;
			}

			// EN: Normal working. Simply send character (escaped or not) to GPRS shield
			// FR: Fonctionnement normal. Envoyer le caractere (echappé ou non) vers GPRS shield
			mySerial.print(SerialInByte);
		}

	}
	else  if (mySerial.available())
	{
		// EN: Listen to GRPS shield + relay to Arduino Serial Monitor 
		// FR: Ecouter les envois du GPRS shield + Renvoi vers Arduino Serial Monitor
		char c = mySerial.read();
		Serial.print(c);
	}

}