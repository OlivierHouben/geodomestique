/*
Name:		IOT_V3.ino
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
//#define _DEBUG_
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
SoftwareSerial Serial1(8, 7);
#endif

// Include Emon Library
#include "EmonLib.h"

// uncomment line for debug


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
unsigned long time_now = 0;
unsigned long period = 120000;

unsigned long previoustime = 0;
unsigned long currenttime = 0;

unsigned int interval = 30000;

// Initialize interupt
int positionOFF = -1;
int positionON = -1;

int volatile voltage1;
int volatile sendemail1;
bool volatile emailsent;

String txtsms;
char* numAuth[] = { "+num1","+num2" };

//TinyGsm modem(SerialAT);
//TinyGsmClient client(modem);
ThingerTinyGSM thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL, Serial1);
TinyGsm modemS = thing.getTinyGsm();
// the setup function runs once when you press reset or power the board
void setup() {

	// Initializing serial commmunication
	Serial.begin(9600);
	// Serial for AT commands (can be higher with HW Serial, or even lower in SW Serial)
	Serial1.begin(9600);
	delay(5000);
	
	// set APN (you can remove user and password from call if your apn does not require them)
	thing.setAPN(APN_NAME, APN_USER, APN_PSWD);

	String imei = modemS.getIMEI();
	Serial.print("imei : ");
	Serial.println(imei);
	// set PIN (optional)
	// thing.setPIN(CARD_PIN);

	Serial1.print("AT+CMGF=1\r");
	delay(200);
	// AT command to set module to send SMS data to serial out upon receipt 
	Serial1.print("AT+CNMI=2,1,0,0,0\r");
	delay(200);

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
		//out["NRJ1"] = kilos[0];
		//out["NRJ2"] = kilos[1];
		//out["NRJ3"] = kilos[2];
		out["NRJT"] = kilos[3];
		//out["Irms1"] = Irmstableau[0];
		//out["Irms2"] = Irmstableau[1];
		//out["Irms3"] = Irmstableau[2];
		//out["Vrms"] = Vrmstableau[0];
	};

}

// the loop function runs over and over again until power down or reset
void loop() {

	thing.handle();
	if (millis() - time_now > period)
	{
		//thing.stop();
		int cpt = 0;
		int nbrSMSRec;
		String smsrecu;

		readPhase();

		pson data;
		data["ID_Meter"] = Idcompteur;
		data["NRJTot(kwh)"] = kilos[3];
		
		Serial.println("read");
		//Serial1.print("AT+CMGF=1\r");

		nbrSMSRec = GetNbrSmsReceived();

		while (cpt < nbrSMSRec)
		{
			smsrecu = GetSimSms(cpt + 1);
			Serial.println(smsrecu);
			cpt++;
		}

		//delay(100);

		//thing.handle();
		//thing.write_bucket("BucketBaw002", data);

		time_now = millis();
		//Serial1.print("AT+CNMI=2,2,0,0,0\r");
		//delay(100);
		modemS.sendSMS("+num1", smsrecu);

		//thing.handle();
	}
	

	/*if (voltage1>150 & emailsent == 0)
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


int GetNbrSmsReceived()
{
	String allsmsrec;
	int CMGLPosL;
	int virCMGLLPos;
	String CMGLStringL;
	int CMGLIntL;

	// AT Cmd to get all received sms
	Serial1.println("AT+CMGL=\"ALL\"");
	allsmsrec = Serial1.readString();

	CMGLPosL = allsmsrec.lastIndexOf("CMGL:");
	virCMGLLPos = allsmsrec.indexOf(",", CMGLPosL);
	CMGLStringL = allsmsrec.substring(CMGLPosL + 6, virCMGLLPos);
	CMGLIntL = CMGLStringL.toInt();
	return CMGLIntL;
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
	Serial1.println(ATCMGR + (posSim));
	TextSmsI = Serial1.readString();
	
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
