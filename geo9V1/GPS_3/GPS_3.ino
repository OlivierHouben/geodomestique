/*
Name:		GPS_1.ino
Created:	06/05/2018 11:24:41
Author:	Florian
*/

#define TINY_GSM_MODEM_SIM800
#define SerialMon Serial

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ArduinoHttpClient.h>
#include <TinyGsmClient.h>
#include "EmonLib.h"

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
unsigned long time_now = 0;
unsigned long period = 60000;

unsigned long previoustime = 0;
unsigned long currenttime = 0;

unsigned int interval = 30000;

// Initialize interupt
int positionOFF = -1;
int positionON = -1;

String txtsms;
String SmsInfoText;
String SmsInfoNum;
String SmsInfoTime;
String SmsInfoId;
String SmsInfoAuthorized;
char* numAuth[] = { "+num1","+num2" };

float flat = 0.00f;
float flon = 0.00f;

static const int RXPin = 13, TXPin = 12;
static const uint32_t GPSBaud = 9600;
int doitonces = 0;

//#define ss Serial1

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial SerialAT(11, 10);
SoftwareSerial ss(13, 12);

const char apn[] = "gprs.base.be";
const char user[] = "base";
const char pass[] = "base";

const char server[] = "geoelec-vault.herokuapp.com";
const int  port = 80;
String res = "/generator_frame_i_mega/?";
String imei;
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
HttpClient http(client, server, port);

void setup()
{
	SerialMon.begin(9600);
	while (!SerialMon) {
		; // wait for serial port to connect. Needed for native USB port only
	}
	//delay(10);

	// Set GSM module baud rate
	ss.begin(9600);
	SerialAT.begin(9600);
	delay(3000);

	// Restart takes quite some time
	// To skip it, call init() instead of restart()
	SerialMon.println(F("Initializing modem..."));
	modem.restart();

	String modemInfo = modem.getModemInfo();
	SerialMon.print(F("Modem: "));
	SerialMon.println(modemInfo);

	imei = modem.getIMEI();
	SerialMon.print(F("Imei: "));
	SerialMon.println(imei);

	// AT command to set module to SMS mode
	SerialAT.print("AT+CMGF=1\r");
	delay(100);
	// AT command to set module to send SMS data to serial out upon receipt 
	SerialAT.print("AT+CNMI=2,1,0,0,0\r");
	delay(200);

	//getImei();
	SerialMon.println("test");
	// Voltage: input pin, calibration, phase_shift
	emon1.voltage(A4, 116.26, 1.7);

	// Current: input pin, calibration.
	emon1.current(emoncurrentPins[0], 29.1);
	emon2.current(emoncurrentPins[1], 29.1);
	emon3.current(emoncurrentPins[2], 29.1);

	previoustime = millis();
	startMillis = millis();

	// Attention pas d'espace dans les champs d'envoie sinon message d'erreur il faut supprimer les espaces des mails qu'on va envoyer � la bd


	// Unlock your SIM card with a PIN
	//modem.simUnlock("1234");
}

void loop()
{
	

	while (true) {

		if (millis() - time_now > period)
		{
			ss.listen();
			getGpsPos();
			SerialAT.listen();
			time_now = millis();
		}
		
		if (SerialAT.available() > 0)
		{
			// Listen to GRPS shield + relay to Arduino Serial Monitor 
			String c = SerialAT.readString();
			Serial.println(c);
		}
	}

	/*
	if (doitonces == 0)
	{
	doitonces = 2;
	getGpsPos();
	}
	*/

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

void CreateFrame()
{
	String idg = imei;
	String r1 = "true";
	String r2 = "false";
	String mag = "true";
	//String lat = "test%20lat";
	//String lon = "test%20lon";
	String pay = "true";
	float lattest = 30.414;
	float lontest = 55.119;
	//generator_frame_test_i
	res = "/generator_frame_i_mega/?";
	res.concat("idg=");
	res.concat(idg);
	res.concat("&nrj=");
	res.concat(kilos[3]);
	res.concat("&p1=");
	res.concat(kilos[0]);
	res.concat("&p2=");
	res.concat(kilos[1]);
	res.concat("&p3=");
	res.concat(kilos[2]);
	res.concat("&c1=");
	res.concat(Irmstableau[0]);
	res.concat("&c2=");
	res.concat(Irmstableau[1]);
	res.concat("&c3=");
	res.concat(Irmstableau[2]);
	res.concat("&vol=");
	res.concat(Vrmstableau[0]);
	res.concat("&r1=");
	res.concat(r1);
	res.concat("&r2=");
	res.concat(r2);
	res.concat("&mag=");
	res.concat(mag);
	res.concat("&lat=");
	res.concat(lattest);
	res.concat("&lon=");
	res.concat(lontest);
	res.concat("&pay=");
	res.concat(pay);

	SerialMon.println(res);
}

String GetNbrSmsReceived()
{
	String allsmsrec;
	int CMGLPosL;
	int virCMGLLPos;
	String CMGLStringL;
	int CMGLIntL;

	// AT Cmd to get all received sms
	SerialAT.println("AT+CMGL=\"ALL\"");
	allsmsrec = SerialAT.readString();

	CMGLPosL = allsmsrec.lastIndexOf("CMGL:");
	virCMGLLPos = allsmsrec.indexOf(",", CMGLPosL);
	CMGLStringL = allsmsrec.substring(CMGLPosL + 6, virCMGLLPos);
	CMGLIntL = CMGLStringL.toInt();
	return allsmsrec;
	//return CMGLIntL;
}

String GetSimSms(int posSim)
{
	String ATCMGR = "AT+CMGR=";
	String TextSmsI;
	int CMGRNumPosStart;
	int CMGRNumPosEnd;
	String CMGRNum;
	int CMGRSmsPosStart;
	int CMGRSmsPosEnd;
	String CMGRSms;

	// ATM Cmd to read SMS num i
	SerialAT.println(ATCMGR + (posSim));
	TextSmsI = SerialAT.readString();

	// Get phone number
	CMGRNumPosStart = TextSmsI.indexOf("READ\",\"");
	CMGRNumPosEnd = TextSmsI.indexOf("\"", CMGRNumPosStart + 7);
	CMGRNum = TextSmsI.substring(CMGRNumPosStart + 7, CMGRNumPosEnd);
	//Serial.println(CMGRNum);

	if (NumAuthorized(CMGRNum) == 1)
	{
		// Get Sms
		CMGRSmsPosStart = TextSmsI.indexOf(":", 20);
		CMGRSms = TextSmsI.substring(CMGRSmsPosStart + 12);
		//Serial.println(CMGRSms);
		return CMGRSms;
	}
	else
	{
		return "NotAuthorized";
	}


}

int NumAuthorized(String number)
{
	for (int i = 0; i < int((sizeof(numAuth)) / (sizeof(*numAuth))); i++)
	{
		if (number == numAuth[i])
		{
			return 1;
		}
	}
	return 0;
}


void ActivateDesactivateRelay(String msg)
{
	int OnPos = -1;
	int OffPos = -1;
	msg.toLowerCase();
	if (msg.indexOf("rel1") > 0)
	{
		OnPos = msg.lastIndexOf("on");
		OffPos = msg.lastIndexOf("off");

		if (OnPos>OffPos && OnPos >= 0) {
			// Turn on relay and save current state
			digitalWrite(relay, HIGH);
			Serial.println("Relay 1 set to ON");
		}
		else if (OffPos>OnPos && OffPos >= 0) {
			// Turn off relay and save current state
			digitalWrite(relay, LOW);
			Serial.println("Relay 1 set to OFF");
		}
	}

	if (msg.indexOf("rel2") > 0)
	{
		OnPos = msg.lastIndexOf("on");
		OffPos = msg.lastIndexOf("off");

		if (OnPos>OffPos && OnPos >= 0) {
			// Turn on relay and save current state
			digitalWrite(relay, HIGH);
			Serial.println("Relay 2 set to ON");
		}
		else if (OffPos>OnPos && OffPos >= 0) {
			// Turn off relay and save current state
			digitalWrite(relay, LOW);
			Serial.println("Relay 2 set to OFF");
		}
	}

	if (msg.indexOf("rel3") > 0)
	{
		OnPos = msg.lastIndexOf("on");
		OffPos = msg.lastIndexOf("off");

		if (OnPos>OffPos && OnPos >= 0) {
			// Turn on relay and save current state
			digitalWrite(relay, HIGH);
			Serial.println("Relay 3 set to ON");
		}
		else if (OffPos>OnPos && OffPos >= 0) {
			// Turn off relay and save current state
			digitalWrite(relay, LOW);
			Serial.println("Relay 3 set to OFF");
		}
	}

}

void SendState(String msg)
{
	msg.toLowerCase();
	if (msg.indexOf("state") >= 0)
	{
		// Send data
		//modem.sendSMS("+", "Data");

	}
}




void GetSimSmsInfo(int posSim)
{
	String ATCMGR = "AT+CMGR=";
	String TextSmsI;
	int CMGRNumPosStart;
	int CMGRNumPosEnd;
	int CMGRTimePosStart;
	int CMGRTimePosEnd;
	int CMGRSmsPosStart;
	int CMGRSmsPosEnd;

	// ATM Cmd to read SMS num i
	SerialAT.println(ATCMGR + (posSim));
	TextSmsI = SerialAT.readString();

	// Get phone number
	CMGRNumPosStart = TextSmsI.indexOf("READ\",\"");
	CMGRNumPosEnd = TextSmsI.indexOf("\"", CMGRNumPosStart + 7);
	SmsInfoNum = TextSmsI.substring(CMGRNumPosStart + 7, CMGRNumPosEnd);

	// Get Time
	CMGRTimePosStart = TextSmsI.indexOf(":", CMGRNumPosStart);
	CMGRTimePosEnd = TextSmsI.indexOf("\"", CMGRTimePosStart);
	SmsInfoTime = TextSmsI.substring(CMGRTimePosStart - 11, CMGRTimePosEnd);

	// Get Sms
	CMGRSmsPosStart = TextSmsI.indexOf(":", 20);
	// CMGRSmsPosEnd = TextSmsI.indexOf("\r\n\r\nOK\r\n");
	CMGRSmsPosEnd = TextSmsI.indexOf("\r\n\r\nOK");
	//Serial.print(CMGRSmsPosEnd);
	SmsInfoText = TextSmsI.substring(CMGRSmsPosStart + 12, CMGRSmsPosEnd);

	if (NumAuthorized(SmsInfoNum) == 1)
	{
		SmsInfoAuthorized = "Ok";
	}

	else
	{
		SmsInfoAuthorized = "None";
	}
	//Serial.println(SmsInfoNum);
	//Serial.println(SmsInfoTime);
	//Serial.println(SmsInfoText);
	//Serial.println(SmsInfoAuthorized);

	Serial.println("Start;" + SmsInfoId + ";" + SmsInfoTime + ";" + SmsInfoNum + ";" + SmsInfoText + ";" + SmsInfoAuthorized + ";Stop");
}

void getImei()
{
	//delay(500);
	String txtImei;
	int ImeiStart;
	int ImeiEnd;
	int tokenImei = 0;
	while (tokenImei != 1)
	{
		if (SerialAT.available())
		{
			txtImei = SerialAT.readString();
			SerialAT.println("AT+CGSN");
			txtImei = SerialAT.readString();
			// CMGRSmsPosEnd = TextSmsI.indexOf("\r\n\r\nOK\r\n");
			ImeiStart = txtImei.indexOf("AT+CGSN");
			ImeiEnd = txtImei.indexOf("\r\n\r\nOK");
			if (ImeiStart == -1)
			{
				SmsInfoId = txtImei.substring(0, ImeiEnd);
			}
			else
			{
				SmsInfoId = txtImei.substring(ImeiStart + 11, ImeiEnd);
			}
			tokenImei = 1;
		}
	}


	//Serial.println("totest");
	//Serial.println(txtImei);
	//Serial.println("endtest");

}

// Send this tram when new sms received 
// +CMTI: "SM", 4
void NotifNewMessage()
{
	String txtNotif;
	String PosNewSms;
	int possms;
	int CMTIPosStart;
	int CMTIPosEnd;
	if (SerialAT.available())
	{
		txtNotif = SerialAT.readString();
		CMTIPosStart = txtNotif.indexOf("+CMTI:");
		CMTIPosEnd = txtNotif.indexOf(",");
		if (CMTIPosStart != -1)
		{
			PosNewSms = txtNotif.substring(CMTIPosStart + 12, CMTIPosEnd + 3);
			possms = PosNewSms.toInt();
		}
	}
}

void DeleateSimSms()
{
	SerialAT.println("AT+CMGD=0,1");
}


int freeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}


void HTTPRequest(int selectreq)
// selectreq = 0 --> frame
// selectreq = 1 --> sms send operator
// selectreq = 2 --> sms received operator
// selectreq = 3 --> sms send generator
// selectreq = 4 --> sms received generator
{
	SerialMon.print(F("Performing HTTP GET request... "));
	int err;
	if (selectreq == 0)
	{
		err = http.get(res);
	}

	if (err != 0) {
		SerialMon.println(F("failed to connect"));
		delay(10000);
		return;
	}

	int status = http.responseStatusCode();
	SerialMon.println(status);
	if (!status) {
		delay(10000);
		return;
	}

	String body = http.responseBody();
	SerialMon.println(F("Response:"));
	SerialMon.println(body);

	http.stop();
	SerialMon.println(F("Server disconnected"));
}


void getGpsPos()
{
	int doitonce = 0;
	while (doitonce != 1)
	{
		while (ss.available() > 0)
		{
			gps.encode(ss.read());
		}
		if (gps.satellites.isValid() && gps.satellites.value() != 0)
		{
			doitonce = 1;

			SerialMon.println(gps.location.lat(), 6);
			SerialMon.println(gps.location.lng(), 6);
			SerialMon.println(gps.location.age());

			SerialMon.println(gps.satellites.value());
			SerialMon.println(gps.satellites.age());

			if (gps.location.age() > 1000 || gps.satellites.age() > 1000)
			{
				doitonce = 0;
			}
		}

		else
		{
			SerialMon.println("Non valid");
			//SerialMon.println(gps.satellites.isValid());
		}
	}
}