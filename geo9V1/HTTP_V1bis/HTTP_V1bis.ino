/**************************************************************
*
* This sketch connects to a website and downloads a page.
* It can be used to perform HTTP/RESTful API calls.
*
* For this example, you need to install ArduinoHttpClient library:
*   https://github.com/arduino-libraries/ArduinoHttpClient
*   or from http://librarymanager/all#ArduinoHttpClient
*
* TinyGSM Getting Started guide:
*   http://tiny.cc/tiny-gsm-readme
*
* For more HTTP API examples, see ArduinoHttpClient library
*
**************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_ESP8266

// Increase RX buffer if needed
//#define TINY_GSM_RX_BUFFER 512

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

// Uncomment this if you want to see all AT commands
//#define DUMP_AT_COMMANDS

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(8, 7); // RX, TX


							   // Your GPRS credentials
							   // Leave empty, if missing user or pass
const char apn[] = "gprs.base.be";
const char user[] = "base";
const char pass[] = "base";

// Server details
// rqt http://immense-fortress-62867.herokuapp.com/db15/?id=6&nom=arduino

const char server[] = "geoelec-vault.herokuapp.com";
//const char server[] = "immense-fortress-62867.herokuapp.com";
// don't support space in send received 505 message error si space put %20
//const char resource[] = "/db15/?id=77&nom=test%20arduino%20ap";
//const String resource = "/generator_frame_test_i/?idg=323test&nrj=41.11&p1=10.57&p2=3&p3=0.514&c1=6.4&c2=7&c3=8&vol=10.4&r1=true&r2=true&mag=true&lat=tlt&lon=tlg&pay=true";
const int  port = 80;
String res = "/generator_frame_test_i/?";


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);
HttpClient http(client, server, port);

void setup() {
	// Set console baud rate
	SerialMon.begin(9600);
	delay(10);

	// Set GSM module baud rate
	SerialAT.begin(9600);
	delay(3000);

	// Restart takes quite some time
	// To skip it, call init() instead of restart()
	SerialMon.println(F("Initializing modem..."));
	modem.restart();

	String modemInfo = modem.getModemInfo();
	SerialMon.print(F("Modem: "));
	SerialMon.println(modemInfo);

	// Attention pas d'espace dans les champs d'envoie sinon message d'erreur il faut supprimer les espaces des mails qu'on va envoyer à la bd

	String idg = "arduinotest";
	float nrjtot = 33;
	float p1 = 12.412;
	float p2 = 14.412;
	float p3 = 17.412;
	float c1 = 11.412;
	float c2 = 1.412;
	float c3 = 6.412;
	float vol = 410;
	String r1 = "true";
	String r2 = "true";
	String mag = "true";
	String lat = "test%20lat";
	String lon = "test%20lon";
	String pay = "true";

	res.concat("idg=");
	res.concat(idg);
	res.concat("&nrj=");
	res.concat(nrjtot);
	res.concat("&p1=");
	res.concat(p1);
	res.concat("&p2=");
	res.concat(p2);
	res.concat("&p3=");
	res.concat(p3);
	res.concat("&c1=");
	res.concat(c1);
	res.concat("&c2=");
	res.concat(c2);
	res.concat("&c3=");
	res.concat(c3);
	res.concat("&vol=");
	res.concat(vol);
	res.concat("&r1=");
	res.concat(r1);
	res.concat("&r2=");
	res.concat(r2);
	res.concat("&mag=");
	res.concat(mag);
	res.concat("&lat=");
	res.concat(lat);
	res.concat("&lon=");
	res.concat(lon);
	res.concat("&pay=");
	res.concat(pay);
	//SerialMon.println(res);

	// Unlock your SIM card with a PIN
	//modem.simUnlock("1234");
}

void loop() {
	SerialMon.print(F("Waiting for network..."));
	if (!modem.waitForNetwork()) {
		SerialMon.println(" fail");
		delay(10000);
		return;
	}
	SerialMon.println(" OK");

	SerialMon.print(F("Connecting to "));
	SerialMon.print(apn);
	if (!modem.gprsConnect(apn, user, pass)) {
		SerialMon.println(" fail");
		delay(10000);
		return;
	}
	SerialMon.println(" OK");

	SerialMon.print(F("Performing HTTP GET request... "));
	int err = http.get(res);
	http.stop();
	SerialMon.print(F("Performing HTTP GET request... "));
	err = http.get(res);
	/*
	if (err != 0) {
	SerialMon.println(F("failed to connect"));
	delay(10000);
	return;
	}
	*/

	//La methode post ne fonctionne pas

	//SerialMon.print(F("Performing HTTP POST request... "));
	// test 1 NOK
	//http.post("/db19/?id=17&nom=arduino");
	/*
	int status = http.responseStatusCode();
	SerialMon.println(status);
	if (!status) {
	delay(10000);
	return;
	}

	while (http.headerAvailable()) {
	String headerName = http.readHeaderName();
	String headerValue = http.readHeaderValue();
	//SerialMon.println(headerName + " : " + headerValue);
	}
	*/
	/*
	int length = http.contentLength();
	if (length >= 0) {
	SerialMon.print(F("Content length is: "));
	SerialMon.println(length);
	}
	if (http.isResponseChunked()) {
	SerialMon.println(F("The response is chunked"));
	}
	*/
	/*
	String body = http.responseBody();
	SerialMon.println(F("Response:"));
	SerialMon.println(body);
	*/
	/*
	SerialMon.print(F("Body length is: "));
	SerialMon.println(body.length());
	*/

	// Second request
	/*
	SerialMon.print(F("Performing HTTP GET request... "));
	err = http.get(res);

	if (err != 0) {
	SerialMon.println(F("failed to connect1"));
	delay(10000);
	return;
	}
	*/
	//if we want to have the answer we have to wait the return to do that we use that :
	/*
	while (http.headerAvailable()) {
	String headerName = http.readHeaderName();
	String headerValue = http.readHeaderValue();
	//SerialMon.println(headerName + " : " + headerValue);
	}
	*/

	// So we can listen the answer with that :
	/*
	String body = http.responseBody();
	SerialMon.println(F("Response:"));
	SerialMon.println(body);
	*/

	// Shutdown

	http.stop();
	SerialMon.println(F("Server disconnected"));

	modem.gprsDisconnect();
	SerialMon.println(F("GPRS disconnected"));

	// Do nothing forevermore
	while (true) {
		delay(1000);
	}
}
