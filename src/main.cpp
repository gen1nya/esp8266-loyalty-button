
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "HTTP.h"
#include <ESP8266TrueRandom.h>
#include <EEPROMAnything.h>
#include <string.h>

#define DEBUG true

#define SETTING_SIZE(a)
#define TOKEN_SIZE(b)
#define PASSWORD_SIZE(c)
#define SSID_SIZE(d)

#define SETTING_MODE  0
#define NORMAL_MODE   1

#define PORT 3004
#define SETTING_SERVER_PORT 80

#define UART_BAUNDRATE 9600
#define WIFI_TIMEOUT 60000
#define UART_TIMEOUT 60000
#define HTTP_TIMEOUT 5000

#define positive_event_code 0x05
#define negative_event_code 0x05
#define setting_event_code 0x03


const char* auth_method = "/api/simpleButtonAuth/";
const char* token = "79d6ba992f14cf93318003642d5954a1";
const char* host = "83.142.166.49";
String ssid_WIFI_AP = "button";
String pass_WIFI_AP= "button00";

String settingPage = "";
String settingCompletePage = "";

unsigned long timeout = 0l;
//String ssid = "";
//String pass = "";
//char* session_token = "52c897b6145a3c511c77c425ddcc31ba9d0cd3edd4b1df652df00770cfdd7a65c70bc392cbd8c61b87db7322e24ccc91a852eef2741edd7deb909b87455cfb499b662d9ba0bb8fe9fdb2e6dafda35d0fdafac5befab7e4fbecafbd8523ffbf56e24482e8ff306be9b317887c";
//bool is_setting ;


ESP8266WebServer server(SETTING_SERVER_PORT);

struct Econfig {
  String ssid;
  String pass;
  char* session_token = "52c897b6145a3c511c77c425ddcc31ba9d0cd3edd4b1df652df00770cfdd7a65c70bc392cbd8c61b87db7322e24ccc91a852eef2741edd7deb909b87455cfb499b662d9ba0bb8fe9fdb2e6dafda35d0fdafac5befab7e4fbecafbd8523ffbf56e24482e8ff306be9b317887c";
  bool is_setting;
} ;
struct Econfig Config ;




void sleep(void);
void onSettingMode(void);
void onNormalMode(char uartChar);
void log(String text);

void getEEPROM(void);
void saveEEPROM(void);

void buildSettingCompletePage(){
  settingCompletePage += "<!DOCTYPE HTML>";
  settingCompletePage += "<html>";
  settingCompletePage +=  "<head>";
  settingCompletePage +=   "<meta charset=\"utf-8\">";
  settingCompletePage +=   "<title>SETTING</title>";
  settingCompletePage +=  "</head>";
  settingCompletePage +=  "<body>";
  settingCompletePage +=   "<h1> Устройство настроено, вы можете закрыть страницу";
  settingCompletePage +=   "</h1>";
  settingCompletePage +=  "</body>";
  settingCompletePage += "</html>";
}

void buildSettingPage(){
  settingPage += "<!DOCTYPE HTML>";
  settingPage += "<html>";
  settingPage +=  "<head>";
  settingPage +=   "<meta charset=\"utf-8\">";
  settingPage +=   "<title>SETTING</title>";
  settingPage +=  "</head>";
  settingPage +=  "<body>";
  settingPage +=  "<form name=\"setting\" method=\"get\" action=\"/save\">";
  settingPage +=   "<p><b>Введите название и пароль вашей сети</b><br>";
  settingPage +=    "<input type=\"text\" name=\"ssid_wifi\"></br>";
  settingPage +=    "<input type=\"password\" name=\"password_wifi\" pattern=\"[A-Za-z0-9]{6,}\">";
  settingPage +=   "</p>";
  settingPage +=   "<p><input type=\"submit\" value=\"Сохранить\"></p>";
  settingPage +=  "</form>";
  settingPage +=  "</body>";
  settingPage += "</html>";
}

void setup() {
  #ifdef DEBUG
  delay(5000);
  #endif
  buildSettingPage();
  buildSettingCompletePage();
  WiFi.disconnect();
  Serial.begin(UART_BAUNDRATE);
  getEEPROM();
  log(Config.ssid);
  log(Config.pass);
  log(Config.is_setting);
  log(Config.session_token);
  if (Config.is_setting)  {
    log("is setting");
    onSettingMode();
  } else{
    if(Config.is_setting==false) {
      log("is normal mode");
      log("waiting STM");
      timeout = millis();
      while(!Serial.available()){
        if (millis() - timeout > UART_TIMEOUT) {
          log("STM time out");
          sleep();
        }
        Serial.print(".");
        delay(50);
      }
      char inChar = (char)Serial.read();
      log("STM data received!");
      delay(50);

      if (inChar == setting_event_code){
        log("received 0x03, go to setting");
        onSettingMode();
      } else {
        log("received not 0x03, normal mode");
        onNormalMode(inChar);
      }
    }
    log("is setting");
    onSettingMode();//сюда пойдём потому что  в еепром стм скорей всего  ерунда каая-нибуль изначально записана
  }
}
void loop() {// вообще мы не должны сюда попасть, но вдруг
  delay(5000);
  log("fuck...");
  ESP.reset();
}

void onNormalMode(char uartChar){
  log("connecting wifi");
  log("10 second delay because arduino is fucking shit desu");
  WiFi.disconnect();
  delay(10000);
  WiFi.begin(Config.ssid.c_str() ,Config.pass.c_str());
  unsigned long timeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
     if (millis() - timeout > WIFI_TIMEOUT) {
       log("WIFI connection timeout");
       sleep();
     }
     delay(500);
     Serial.print(".");
  }
  log("connected");

  WiFiClient client;
  if (!client.connect(host, PORT)) {
     log("connection failed");
     return;
  }


  String url = "";
  if (uartChar == positive_event_code) url = "/api/%F0%9F%99%82/";
  if (uartChar == negative_event_code) url = "/api/%F0%9F%98%A1/";
  url += "?token=";
  url += Config.session_token;

  log("Requesting URL: ");
  log(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Connection: close\r\n\r\n");
  timeout = millis();
  while (client.available() == 0) {
     if (millis() - timeout > HTTP_TIMEOUT) {
       log("Client Timeout !");
       client.stop();
       return;
     }
  }

  while(client.available()){
     String line = client.readStringUntil('\r');
     log(line);
  }

  delay(1000);
  log("disconnect wifi");
  WiFi.disconnect();
  delay(1000);
  log("sleep");
  sleep();
}


void auth(){
  log("auth");
  WiFiClient client;
  if (!client.connect(host, PORT)) {
     log("connection failed");
     return;
  }

  String url = "";
  url = auth_method;
  //url += "?token=";
  url += token;

  log("Requesting URL: ");
  log(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Connection: close\r\n\r\n");
  timeout = millis();
  while (client.available() == 0) {
     if (millis() - timeout > HTTP_TIMEOUT) {
       log("Client Timeout !");
       client.stop();
       return;
     }
  }

  while(client.available()){
     String line = client.readStringUntil('\r');
     log(line);
  }
}

void handleRoot() { server.send(200, "text/html", settingPage); }

void handleSetting() { server.send(200, "text/html", settingCompletePage);
  if(server.hasArg("ssid_wifi")){
    Config.ssid = server.arg("ssid_wifi");
    log(Config.ssid);
  }
  if(server.hasArg("password_wifi")){
    Config.pass = server.arg("password_wifi");
    log(Config.pass);

  }
Config.is_setting = false;
   saveEEPROM();
}

void onSettingMode(){
    Config.is_setting = true;
    log("enable wifi AP");
    WiFi.mode (WIFI_AP);
    WiFi.begin(ssid_WIFI_AP.c_str(), pass_WIFI_AP.c_str());
    WiFi.softAP(ssid_WIFI_AP.c_str(), pass_WIFI_AP.c_str());
    log(WiFi.softAPIP());
    server.on("/", handleRoot);
    server.on("/save", handleSetting);
    server.begin();
    log("HTTP server started");
    while(Config.is_setting){
      server.handleClient();
    }
}

void getEEPROM(){
  Serial.write(0x01);// read
  Serial.print(0x01);// key setting
  while(!Serial.available()){
    log("Wait STM");
  }
  while(Serial.available()>0){
  SETTING_SIZE((uint8_t)Serial.read())
  }
  while(!Serial.available()){
    log("Wait STM");
  }
  while(Serial.available()>0){
  Config.is_setting=(bool)Serial.read();
  }
  log(Config.is_setting);

  Serial.write(0x01);// read
  Serial.print(0x02);// key setting
  while(!Serial.available()){
    log("Wait STM");
  }
  while(Serial.available()>0){
  SSID_SIZE((uint8_t)Serial.read())
  }
  while(!Serial.available()){
    log("Wait STM");
  }
  while (Serial.available()>0){
    Config.ssid+=(String)Serial.read();
    }
  log(Config.ssid);

  Serial.write(0x01);// read
  Serial.print(0x03);// PASS setting
  while(!Serial.available()){
    log("Wait STM");
  }
  while(Serial.available()>0){
  PASSWORD_SIZE((uint8_t)Serial.read())
  }
  while(!Serial.available()){
    log("Wait STM");
  }
  while (Serial.available()>0){
    Config.pass+=(String)Serial.read();
    }
  log(Config.pass);

  Serial.write(0x01);// read
  Serial.print(0x04);// TOKEN setting
  while(!Serial.available()){
    log("Wait STM");
  }
  while(Serial.available()>0){
 TOKEN_SIZE((uint8_t)Serial.read())
  }
  while(!Serial.available()){
    log("Wait STM");
  }
  while (Serial.available()>0){
    Config.session_token+=(char)Serial.read();
    }
  log(Config.session_token);
}


void saveEEPROM(){
Serial.print(0x00);//запись
Serial.print(0x01);// key setting
Serial.print(0x01);//1 byte
Serial.print(Config.is_setting);//date
delay(50);
Serial.print(0x00);
Serial.print(0x02);// key ssid
Serial.print(strlen(Config.ssid.c_str()));//size
Serial.print(Config.ssid);//date
delay(50);
Serial.print(0x00);
Serial.print(0x03);//  key pass
Serial.print(strlen(Config.pass.c_str()));//size
Serial.print(Config.pass);//date
delay(50);
Serial.print(0x00);
Serial.print(0x04);// key token
Serial.print(strlen(Config.session_token));//size
Serial.print(Config.session_token);//date
}
/*void getEEPROM() {
    int first = 211;
    EEPROM.get(0, first);
    if(first != 211) {
        log("is first launch, save default data");
        Config.ssid = ssid_WIFI_AP;
        Config.pass = pass_WIFI_AP;
        Config.session_token = "";
        is_setting = true;
        EEPROM.put(0, 211);
        saveEEPROM();
    }
    uint8_t address = 1;
    EEPROM_readAnything(address, ssid);
    address += SSID_SIZE;
    EEPROM_readAnything(address, pass);
    address += PASSWORD_SIZE;
    EEPROM_readAnything(address, session_token);
    address += TOKEN_SIZE;
    EEPROM_readAnything(address, is_setting);
}*/


/*void saveEEPROM() {
  EEPROM.put(0, 211);
  uint8_t address = 1;
  EEPROM_writeAnything(address, ssid);
	address += SSID_SIZE;
  EEPROM_writeAnything(address, pass);
  address += PASSWORD_SIZE;
	EEPROM_writeAnything(address, session_token);
	address += TOKEN_SIZE;
	EEPROM_writeAnything(address, is_setting);
}*/

void sleep(){
  ESP.deepSleep(2000000);
  ESP.reset();
}

void log(String text){
#ifdef DEBUG
    Serial.print(millis());
    Serial.print(": ");
    Serial.println(text);
#endif
}
