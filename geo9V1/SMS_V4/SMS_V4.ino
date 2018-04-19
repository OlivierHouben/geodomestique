#include <SoftwareSerial.h>
char* numAuth[] = { "+num1","+num2" };
SoftwareSerial SerialATM(8, 7);
String txtsms;
int doitonce = 0;
void setup() {
	Serial.begin(9600);
	SerialATM.begin(9600);
	//Serial.println(numAuth[0]);
	//Serial.println(numAuth[1]);
	//Serial.println(numAuth);

}



void loop() {
	// put your main code here, to run repeatedly:
	if (doitonce == 0)
	{
		int cpt = 0;

		int nbrSMSRec;

		String smsrecu;

		nbrSMSRec = GetNbrSmsReceived();


		while (cpt < nbrSMSRec)
		{
			smsrecu = GetSimSms(cpt + 1);
			Serial.println(smsrecu);
			cpt++;
		}

		//Serial.println(txtsms.length());

		doitonce = 1;
		//int possms = txtsms[7] - 48;
		//int possms = txtsms[7] - '0';

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
	SerialATM.println("AT+CMGL=\"ALL\"");
	allsmsrec = SerialATM.readString();

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
