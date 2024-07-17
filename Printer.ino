
#include <ESPmDNS.h>
#include "Variables.h"
#include "WebSocketClient.h"
#include "WiFiConfiguration.h"
#include "WebServer.h"
#include "HttpClient.h"
#define DefPwr true
#define DefSig true

void setup() {

  Serial.begin(115200);
  mac = ESP.getEfuseMac();
  pinMode(AntennaGreen, INPUT_PULLUP);
  pinMode(Relay, OUTPUT);
  pinMode(Connection, OUTPUT);
  pinMode(AntennaRed, INPUT_PULLUP);

  digitalWrite(Relay, HIGH);

  InitPref();
  manifest_url = "http://" + server_IP.toString() + ":" + String(port) + "/firmware";
  InitOTA();
  if (!CreateSTA()) {
    CreateAP();
  }
  MDNS.begin(String(host + mac.substring(mac.length() - 4)));
  Serial.print("Host:");
  Serial.println(host + mac.substring(mac.length() - 4) + ".local");
  WebServices();
  InitWSClient();
  CheckUpdates(false);
}
void loop() {
  if (Enable) {
    if (digitalRead(AntennaRed) == DefSig && digitalRead(AntennaGreen) == DefPwr) {
      state = 0;
      if (state != prestate) {
        webSocket.sendTXT("{\"action\":" + String(Status) + ",\"Alarm\":\"Normal\"}");
      }
      prestate = state;
    } else if (digitalRead(AntennaRed) == DefSig && digitalRead(AntennaGreen) != DefPwr) {
      state = 1;
      if (state != prestate) {
        webSocket.sendTXT("{\"action\":" + String(Status) + ",\"Alarm\":\"Antenna Off\"}");
      }
      prestate = state;
    } else if (digitalRead(AntennaRed) != DefSig && digitalRead(AntennaGreen) == DefPwr) {
      state = 2;
      if (state != prestate  && prestate != 3) {
        webSocket.sendTXT("{\"action\":" + String(Status) + ",\"Alarm\":\"Alert\"}");
      }
      prestate = state;
    } else if (digitalRead(AntennaRed) != DefSig && digitalRead(AntennaGreen) != DefPwr) {
      state = 3;
      if (state != prestate  && prestate != 2) {
        webSocket.sendTXT("{\"action\":" + String(Status) + ",\"Alarm\":\"Alert\"}");
      }
      prestate = state;
    }
  }
  if (UpdateFirmware) {
    Serial.print("Updated:");
    Serial.println(CheckUpdates(true));
    UpdateFirmware = false;
  } else {
    webSocket.loop();
  }
  CheckWifi();
}
