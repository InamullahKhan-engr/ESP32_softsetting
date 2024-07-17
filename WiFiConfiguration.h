#ifndef WIFICONFIGURATION_H
#define WIFICONFIGURATION_H
#include <WiFi.h>

void CreateAP(){
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP((host+ mac.substring(mac.length() - 4) +".local").c_str(), passap.c_str());
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());
  wifilost=millis();
}
bool CreateSTA(){
  WiFi.mode(WIFI_STA);
  Serial.print("\n[*] Connecting to WiFi Network "+ ssidsta);
  WiFi.begin(ssidsta.c_str(), passsta.c_str());
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime >= 10000) {
      return false;
    }
    delay(1000);
    Serial.print(".");
  }
  IPAddress ip = WiFi.localIP();
  Serial.print("\n[+] Connected to WiFi network with local IP : ");
  Serial.println(WiFi.localIP());
  return true;
}
void CheckWifi(){
  if(WiFi.getMode()==WIFI_AP){
    if(WiFi.softAPgetStationNum()==0){
      if(millis()-wifilost>ap_idle_tme*1000){
        if(!CreateSTA()){
          CreateAP();
        }
      }
    }
  }else{
    if(WiFi.status() != WL_CONNECTED){
      CreateAP();
    }
  }
}
#endif
