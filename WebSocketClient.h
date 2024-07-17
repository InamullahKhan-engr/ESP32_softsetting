#include <Base64.h>
#include <global.h>
//#include <MD5.h>
#include <sha1.h>
#include <WebSocketClient.h>


#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H
#include <WebSocketsClient.h>
WebSocketsClient webSocket;
enum Codes {
  Status,
  Version,
  UpdateFW
};
void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
    const uint8_t* src = (const uint8_t*)mem;
    Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++) {
        if (i % cols == 0) {
            Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        Serial.printf("%02X ", *src);
        src++;
    }
    Serial.printf("\n");
}

void ProcessCmd(uint8_t* pl){
  Serial.printf("Payload: %s\n", pl);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, reinterpret_cast<const char*>(pl));

  if (error) {
    Serial.printf("Deserialize error: %s\n", pl);
  }
  switch(doc["action"].as<int>()){
    case Status:
      if(doc["status"].as<String>()=="Enable"){
        Enable=true;
        digitalWrite(Connection,LOW);
        //prestate++;
      }else{
        Enable=false;
        webSocket.sendTXT("{\"action\":"+String(Status)+",\"Alarm\":\"Inactive\"}");
        digitalWrite(Connection,HIGH);
      }
    break;
    case UpdateFW:
      UpdateFirmware=true;
    break;
    default:
      Serial.println("Resp: ");
    break;
  }
}
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch(type) {
    case WStype_DISCONNECTED:
      digitalWrite(Relay,HIGH);
      WSConn=false;
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      digitalWrite(Relay,LOW);
      WSConn=true;
      webSocket.sendTXT("{\"action\":"+String(Version)+",\"Version\":\""+FIRMWARE_VERSION+"\"}");
      Serial.printf("[WSc] Connected!\n");
      break;
    case WStype_TEXT:
      ProcessCmd(payload);
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
    case WStype_ERROR:      
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}
void InitWSClient(){
  Serial.println("Websocket connecting");
  webSocket.begin(server_IP, 3000, "/");
  webSocket.setAuthorization("ESP", mac.c_str());
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

#endif
