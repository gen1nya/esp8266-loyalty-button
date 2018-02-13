#ifndef HTTP_h
#define HTTP_h
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
extern ESP8266WebServer server;
void HTTP_handleRoot(void);
void handleSetting(void);
void buildSettingCompletePage(void);
void buildSettingPage(void);
//extern bool isAP;
//extern bool flag_btn;
//extern int Hum;
//extern int Temp;
//extern int Avalue0;
//extern int Count1;
//extern int Count2;
//extern uint32_t first_tm;
//extern uint32_t tm;

//bool ConnectWiFi(void);
//void HTTP_begin(void);

//void HTTP_handleConfig(void);
//void HTTP_handleConfig2(void);
//void HTTP_handleSave(void);
//void HTTP_handleDefault(void);
//void HTTP_handleReboot(void);
//void HTTP_handleButton(void);
//void HTTP_loop();
//void WiFi_begin(void);
//void Time2Str(char *s,time_t t);
//bool SetParamHTTP();



#endif
