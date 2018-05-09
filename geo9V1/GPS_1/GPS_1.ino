/*
 Name:		GPS_1.ino
 Created:	06/05/2018 11:24:41
 Author:	Florian
*/

#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 13, TXPin = 12;
static const uint32_t GPSBaud = 9600;
int doitonces = 0;
unsigned long time_now = 0;
unsigned long period = 10000;
float flat = 0.00f;
float flon = 0.00f;
char clon[11];
String slat;
String slon;
//#define ss Serial1

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup()
{
	//Serial.begin(115200);
	Serial.begin(9600);
	ss.begin(GPSBaud);
}

void loop()
{
	if (millis() - time_now > period)
	{
		getGpsPos();
		Serial.println(flat,6);
		Serial.println(flon,6);
		//dtostrf(flon, 11, 6, clon);
		//Serial.println(clon);
		Serial.println("");
		slat = String(flat, 6);
		Serial.println(slat);
		slon = String(flon, 6);
		Serial.println(slon);
		time_now = millis();
		Serial.println("");
		Serial.println("");
	}
	/*
	if (doitonces == 0)
	{
		doitonces = 2;
		getGpsPos();
	}
	*/

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

			//Serial.println(gps.location.lat(), 6);
			//Serial.println(gps.location.lng(), 6);
			//Serial.println(gps.location.age());

			//Serial.println(gps.satellites.value());
			//Serial.println(gps.satellites.age());

			flat = gps.location.lat();
			flon = gps.location.lng();

			if (gps.location.age() > 1000 || gps.satellites.age() > 1000)
			{
				doitonce = 0;
			}
		}

		else
		{
			//Serial.println("Non valid");
			//Serial.println(gps.satellites.isValid());
		}
	}
}