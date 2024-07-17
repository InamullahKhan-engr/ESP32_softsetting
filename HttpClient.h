#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H
#include <AsyncTCP.h>


void makeAsyncHttpPostRequest(String endpoint, String postData,AsyncWebServerRequest* req, void (*callback)(JsonDocument doc, AsyncWebServerRequest* request)) {
  AsyncClient* client = new AsyncClient();
  client->onData([postData, req , callback](void* obj, AsyncClient* c, void *data, size_t len) {
    char* receivedData = (char*)data;
    char* headerEnd = strstr(receivedData, "\r\n\r\n");
    if (headerEnd != nullptr) {
      headerEnd += 4;
      size_t bodyLength = len - (headerEnd - receivedData);
      String responseBody = "";
      responseBody.concat(headerEnd, bodyLength);
      Serial.println(responseBody);
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, responseBody);
      if (error) {
        doc.clear();
        doc["status"]="error";
        doc["msg"]="Deserialization Error";
      }
      callback(doc, req);
    }
  }, client);

  client->onConnect([postData, endpoint, callback](void* obj, AsyncClient* c) {
    String request = "POST /esp/" + endpoint + " HTTP/1.1\r\n";
    request += "Host: " + String(server_IP) + "\r\n";
    request += "Content-Type: application/x-www-form-urlencoded\r\n";
    request += "Content-Length: " + String(postData.length()) + "\r\n";
    request += "Connection: close\r\n\r\n";
    request += postData;
    c->write(request.c_str(), request.length());
  }, client);

  client->onDisconnect([callback](void* obj, AsyncClient* c) {
    Serial.println("Closesd");
    delete c;
  }, client);

  client->connect(server_IP, port);
}

#endif
