#include <SoftwareSerial.h>
char* numAuth[] = { "+num1","+num2" };
SoftwareSerial SerialATM(8, 7);
//String txtsms;

String SmsInfoText;
String SmsInfoNum;
String SmsInfoTime;
String SmsInfoId;
String SmsInfoAuthorized;
unsigned long time_now = 0;
unsigned long period = 30000;
void setup() {
	Serial.begin(9600);
	SerialATM.begin(9600);
	//Serial.println(numAuth[0]);
	//Serial.println(numAuth[1]);
	//Serial.println(numAuth);
	SerialATM.print("AT+CMGF=1\r");
	delay(200);
	// AT command to set module to send SMS data to serial out upon receipt 
	SerialATM.print("AT+CNMI=2,1,0,0,0\r");
	delay(200);
	getImei();
}



void loop() {
	// put your main code here, to run repeatedly:
	if (millis() - time_now > period)
	{
		int cpt = 0;

		int nbrSMSRec;

		String smsrecu;

		nbrSMSRec = GetNbrSmsReceived();


		while (cpt < nbrSMSRec)
		{
			GetSimSmsInfo(cpt + 1);
			//smsrecu = GetSimSms(cpt + 1);
			//Serial.println(smsrecu);
			cpt++;
		}

		//Serial.println(txtsms.length());
		time_now = millis();

		DeleateSimSms();

		//int possms = txtsms[7] - 48;
		//int possms = txtsms[7] - '0';
	}

	if (SerialATM.available())
	{
		// Listen to GRPS shield + relay to Arduino Serial Monitor 
		char c = SerialATM.read();
		Serial.print(c);
	}
	
	//NotifNewMessage();
}

int GetNbrSmsReceived()
{
	String allsmsrec;
	int CMGLPosL;
	int virCMGLLPos;
	String CMGLStringL;
	int CMGLIntL;

	// AT Cmd to get all received sms
	SerialATM.println("AT+CMGL=\"ALL\"");
	allsmsrec = SerialATM.readString();

	CMGLPosL = allsmsrec.lastIndexOf("CMGL:");
	virCMGLLPos = allsmsrec.indexOf(",", CMGLPosL);
	CMGLStringL = allsmsrec.substring(CMGLPosL + 6, virCMGLLPos);
	CMGLIntL = CMGLStringL.toInt();
	//Serial.println(CMGLIntL);
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
	SerialATM.println(ATCMGR + (posSim));
	TextSmsI = SerialATM.readString();

	// Get phone number
	CMGRNumPosStart = TextSmsI.indexOf("READ\",\"");
	CMGRNumPosEnd = TextSmsI.indexOf("\"", CMGRNumPosStart + 7);
	CMGRNum = TextSmsI.substring(CMGRNumPosStart + 7, CMGRNumPosEnd);
	//Serial.println(CMGRNum);

	if (NumAuthorized(CMGRNum) == 1)
	{
		// Get Sms
		CMGRSmsPosStart = TextSmsI.indexOf(":", 20);
		// CMGRSmsPosEnd = TextSmsI.indexOf("\r\n\r\nOK\r\n");
		CMGRSmsPosEnd = TextSmsI.indexOf("\r\n\r\nOK");
		//Serial.print(CMGRSmsPosEnd);
		// CMGRSms = TextSmsI.substring(CMGRSmsPosStart + 12);
		CMGRSms = TextSmsI.substring(CMGRSmsPosStart + 12, CMGRSmsPosEnd);
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
	SerialATM.println(ATCMGR + (posSim));
	TextSmsI = SerialATM.readString();

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
		if (SerialATM.available())
		{
			txtImei = SerialATM.readString();
			SerialATM.println("AT+CGSN");
			txtImei = SerialATM.readString();
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
	if (SerialATM.available())
	{
		txtNotif = SerialATM.readString();
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
	SerialATM.println("AT+CMGD=0,1");
}