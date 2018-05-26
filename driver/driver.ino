#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "FS.h"


ESP8266WiFiMulti WiFiMulti;

const int wwPin = D2;
const int cwPin = D3;

const char *ssid;
const char *password;

int wwValue = 0;
int cwValue = 0;
#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

ESP8266WebServer server(80);

void setup() { 
  //Setup pin modes
  pinMode(wwPin, OUTPUT);
  pinMode(cwPin, OUTPUT);

  SPIFFS.begin();

  analogWrite(wwPin, map(wwValue, 0, 100, 0, 1023));
  analogWrite(cwPin, map(cwValue, 0, 100, 0, 1023));

  loadLastValues();

  analogWrite(wwPin, map(wwValue, 0, 100, 0, 1023));
  analogWrite(cwPin, map(cwValue, 0, 100, 0, 1023));
  
  for (uint8_t t = 1; t > 0; t--)
  {
    delay(1000);
  }
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  loadConfig();

  WiFiMulti.addAP(ssid, password);

  int tries = 0;
    while (WiFiMulti.run() != WL_CONNECTED)
    {
      DEBUG_PRINTLN("Try to connect...");
      delay(1000);
      if (tries > 30)
      {
        DEBUG_PRINTLN("Started Acccess Point");
        break;
      }
      tries++;
    }

    DEBUG_PRINTLN(WiFi.localIP());

  startServer();
}

void loop() {
  server.handleClient();
}

void startServer()
{
  server.on("/setww", HTTP_POST, setWw);
  server.on("/setcw", HTTP_POST, setCw);
  server.on("/getww", HTTP_POST, getWw);
  server.on("/getcw", HTTP_POST, getCw);
  server.onNotFound([]() {
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();
}

void setWw()
{
  wwValue = server.arg("value").toInt();
  DEBUG_PRINTLN(wwValue);
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", "Saved");
  setLed();
}
void setCw()
{
  cwValue = server.arg("value").toInt();
  DEBUG_PRINTLN(cwValue);
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", "Saved");
  setLed();
}

void getWw()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  char buf [4];
  sprintf (buf, "%d", wwValue);
  server.send(200, "text/html", buf);
}

void getCw()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  char buf [4];
  sprintf (buf, "%d", cwValue);
  server.send(200, "text/html", buf);
}

void setLed()
{
  analogWrite(wwPin, map(wwValue, 0, 100, 0, 1023));
  analogWrite(cwPin, map(cwValue, 0, 100, 0, 1023));
  saveValues();
}

void loadLastValues()
{
  File configFile = SPIFFS.open("/values.json", "r");
  if (!configFile) {
    return;
  }
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    return;
  }

  wwValue = json["wwValue"].as<int>();
  cwValue = json["cwValue"].as<int>();
}

void saveValues() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["wwValue"] = wwValue;
  json["cwValue"] = cwValue;

  File configFile = SPIFFS.open("/values.json", "w");
  if (!configFile) {
    return;
  }

  json.printTo(configFile);
  return;
}

void loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    return;
  }

  size_t size = configFile.size();

  std::unique_ptr<char[]> buf(new char[size]);
  
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    return;
  }

  ssid = json["ssid"];
  password = json["password"];
}


