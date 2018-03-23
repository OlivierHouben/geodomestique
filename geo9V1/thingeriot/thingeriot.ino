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
SoftwareSerial Serial1(7,8); // RX, TX
#endif

#define USERNAME "houbeno"
#define DEVICE_ID "arduino"
#define DEVICE_CREDENTIAL "sisisi"

								// use your own APN config
#define APN_NAME "internet.proximus.be"
#define APN_USER ""
#define APN_PSWD ""

								// set your cad pin (optional)
#define CARD_PIN ""

ThingerTinyGSM thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL, Serial1);

void setup() {
	// uncomment line for debug
	 Serial.begin(9600);

	// Serial for AT commands (can be higher with HW Serial, or even lower in SW Serial)
	Serial1.begin(9600);

	// set APN (you can remove user and password from call if your apn does not require them)
	thing.setAPN(APN_NAME, APN_USER, APN_PSWD);

	// set PIN (optional)
	// thing.setPIN(CARD_PIN);

	// resource input example (i.e, controlling a digitalPin);
	pinMode(LED_BUILTIN, OUTPUT);
	thing["led"] << digitalPin(LED_BUILTIN);

	// resource output example (i.e. reading a sensor value)
	thing["millis"] >> outputValue(millis());

	// more details at http://docs.thinger.io/arduino/
}

void loop() {
	thing.handle();
}