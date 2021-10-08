#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "LITTLEFS.h"
//#include <wifisetup.h>

const char* ssid = "HOMENET";
const char* haslo = "!@#$5678Dom";
const char* nazwa = "LicznikEn";

const long ILPOM = 500; 

typedef struct 
{ 
	double RMSCurrent = 0;
	double RMSPower = 0;
	double peakPower = 0;
	double kilos = 0;
	double odczyt = 0;
	unsigned long startMillis = 0;
	unsigned long endMillis = 0;
	double dane[ILPOM];
	double kalib = 8912;
	String danestr;

} zapisFazy;

zapisFazy faza[3];

unsigned long licznik;
Adafruit_ADS1115 ads;  
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0, 21600000);

WebServer server(666);
TaskHandle_t taskCzytajHandle = NULL;


String sform(int i)
{
	if(i>9) return String(i); else return "0" + String(i);
}


void zapiszWynik(int f)
{
	faza[f].danestr = "";
	for (int i = ILPOM-1; i > 0; i--)
	{
		faza[f].dane[i] = faza[f].dane[i-1];
		faza[f].danestr += ",";
		faza[f].danestr += String(faza[f].dane[ILPOM - i]);
	}
	faza[f].dane[0] = faza[f].RMSPower;
	faza[f].danestr = String(faza[f].RMSPower) + faza[f].danestr;
}

void readPhase (int f) 
{
    long current = 0;
    long maxCurrent = 0;

    for (int j=0 ; j<= 150 ; j++)  
    {
		current =  ads.readADC_SingleEnded(f+1);
		if(current >= maxCurrent) maxCurrent = current;
    }
	if (maxCurrent <= faza[f].kalib) maxCurrent = faza[f].kalib;

	faza[f].odczyt = maxCurrent;
    faza[f].RMSCurrent = ((maxCurrent - faza[f].kalib)* 0.707)/ 441.19;
	faza[f].RMSPower = 230*faza[f].RMSCurrent;
    if (faza[f].RMSPower > faza[f].peakPower) faza[f].peakPower = faza[f].RMSPower;

    faza[f].endMillis= millis();
    unsigned long time = (faza[f].endMillis - faza[f].startMillis);
    faza[f].kilos +=  ((double)faza[f].RMSPower * ((double)time/60/60/1000000));
	faza[f].startMillis= millis();
	zapiszWynik(f);
}

void czytajFazy(void * parameter)
{
	for(;;)
	{
		if (millis()-licznik > 1000)
		{
			licznik = millis();
			readPhase(0);
			readPhase(1);
			readPhase(2);
		}
	}
}





void handleDane()
{
	timeClient.getFormattedTime();
	
	String str="{\"f1_ampery\":\""   + String(faza[0].RMSCurrent) +
             "\",\"f1_waty\":\""   + String(faza[0].RMSPower) +
			 "\",\"f1_kilos\":\""   + String(faza[0].kilos) +
             "\",\"f1_max\":\""   + String(faza[0].peakPower) +
			 "\",\"f2_ampery\":\""   + String(faza[1].RMSCurrent) +
			 "\",\"f2_waty\":\""   + String(faza[1].RMSPower) +
			 "\",\"f2_kilos\":\""   + String(faza[1].kilos) +
             "\",\"f2_max\":\""   + String(faza[1].peakPower) +
			 "\",\"f3_ampery\":\""   + String(faza[2].RMSCurrent) +
			 "\",\"f3_waty\":\""   + String(faza[2].RMSPower) +
			 "\",\"f3_kilos\":\""   + String(faza[2].kilos) +
			 "\",\"f3_max\":\""   + String(faza[2].peakPower) +
			 "\",\"f1_dane\":[" + faza[0].danestr + "]"
			 ",\"f2_dane\":[" + faza[1].danestr + "]"
			 ",\"f3_dane\":[" + faza[2].danestr + "]}";
	
	server.send(200, "text/json", str);
}

String getContentType(String filename) 
{
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".json")) return "text/json";
    return "text/plain";
}
 
File handleFileRead(String path) 
{
    File file;
    if (LITTLEFS.exists(path)) 
    {
        file = LITTLEFS.open(path, "r");
        return file;
    }
    return file;
}

void handleFile()
{
	String n = server.uri();
	if (n == "/") n = "/index.html";
	
	File f = handleFileRead(n);
	if (f)
	{	
		String contentType = getContentType(n);
		server.streamFile(f, contentType);
	}
	else
	{
		server.send(404, "text/plain", "404:  " + n);	
	}
}

void setup (void) 
{
	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(500);

	Serial.println(F("Inizializing FS..."));
    if (LITTLEFS.begin())
	{
        Serial.println(F("done."));
    }
	else
	{
        Serial.println(F("fail."));
    } 

	if(ssid != "")
	{
		WiFi.hostname(nazwa);
		WiFi.begin(ssid, haslo);
		int i = 0;
		while (WiFi.status() != WL_CONNECTED) 
		{
			delay(500);
			if(i > 100) break;
			i++;
		}
	}

	if (WiFi.status() != WL_CONNECTED)
	{
		WiFi.mode(WIFI_AP);
		
		delay(100);
		IPAddress ip(192,168,99,1);
		IPAddress brama(192,168,99,1);
		IPAddress maska(255,255,255,0);
		
		WiFi.softAP(nazwa);
		WiFi.softAPConfig(ip, brama, maska);
	}
  
	//server.on("/wifi", WIFIsetup);
	server.on("/dane", HTTP_POST, handleDane);
	server.onNotFound(handleFile);
	server.begin();

	timeClient.begin();
	timeClient.forceUpdate();

	ArduinoOTA.setHostname("Liczniken"); 
	ArduinoOTA.begin();

	Wire.begin(15, 14);
	if (!ads.begin()) 
	{
		Serial.println("Problem z ADS.");
	}

	while(timeClient.getEpochTime() < 1000000)
	{
		delay(300);
	}
	
	long mil = millis();
	faza[0].startMillis = mil;
	faza[1].startMillis = mil;
	faza[2].startMillis = mil;

	xTaskCreatePinnedToCore(
             czytajFazy, /* Task function. */
             "czytajFazy",   /* name of task. */
             10000,     /* Stack size of task */
             NULL,      /* parameter of the task */
             1,         /* priority of the task */
             &taskCzytajHandle,    /* Task handle to keep track of created task */
             0);        /* pin task to core 0 */
}

void loop (void) 
{
	ArduinoOTA.handle();
	server.handleClient();
}
