#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <SPIFFS.h>
#include "ota.h"
#include "HttpClient.h"
#include <Update.h>
String convertFileSize(const size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < 1024 * 1024) {
    return String(bytes / 1024.0) + " kB";
  } else if (bytes < 1024 * 1024 * 1024) {
    return String(bytes / 1048576.0) + " MB";
  } else {
    return "0 B";
  }
}

String getFileInformation() {
  String files = "[";
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    //Serial.println(fileName);
    if (files.length() > 1) {
      files += ",";
    }
    files += "{\"" + fileName + "\":\"" + convertFileSize(file.size()) + "\"}";

    file = root.openNextFile();
  }

  root.close();
  files += ",{\"full\":\"" + convertFileSize(SPIFFS.totalBytes()) + "\"}";
  files += ",{\"used\":\"" + convertFileSize(SPIFFS.usedBytes()) + "\"}";
  files += ",{\"free\":\"" + convertFileSize(SPIFFS.totalBytes() - SPIFFS.usedBytes()) + "\"}";
  files += "]";
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, files);
  return files;
}

String getVariables() {
  DynamicJsonDocument jsonDocument(1024);
  jsonDocument["passap"] = passap;
  jsonDocument["ap_ip"] = String(ap_ip0) + "." + String(ap_ip1) + "." + String(ap_ip2) + "." + String(ap_ip3);
  jsonDocument["server_ip"] = String(server_ip0) + "." + String(server_ip1) + "." + String(server_ip2) + "." + String(server_ip3);
  jsonDocument["ap_idle_tme"] = ap_idle_tme;
  jsonDocument["port"] = port;
  jsonDocument["ssidsta"] = ssidsta;
  jsonDocument["passsta"] = passsta;
  jsonDocument["mac"] = mac;
  jsonDocument["msg"] = msg;
  jsonDocument["host"] = host;
  
  if (FirmwareVersion != "") {
    jsonDocument["vesrsioninfo"] = "Current Ver: " + String(FIRMWARE_VERSION) + "   New Update Ver: " + String(FirmwareVersion);
  } else {
    jsonDocument["vesrsioninfo"] = "Current Ver: " + String(FIRMWARE_VERSION);
  }
  jsonDocument["files"]=getFileInformation();
  String jsonString;
  serializeJson(jsonDocument, jsonString);
  return jsonString;
}
void doActionAndSendResponse(AsyncWebServerRequest* request, const char* action, const char* contentType, String str) {
  String jsonString = getVariables();
  String contentTyp = "application/json";
  AsyncWebServerResponse *response = request->beginResponse(200, contentTyp, jsonString);
  response->addHeader("X-Identification", str);
  request->send(response);
}

String createJson(String titles, String statuss, String msgs) {
  StaticJsonDocument<512> setDoc;
  String setJsonString = getVariables();
  DeserializationError error = deserializeJson(setDoc, setJsonString);
  StaticJsonDocument<1024> doc;
  JsonObject set = doc.createNestedObject("set");
  set.set(setDoc.as<JsonObject>());
  JsonObject action = doc.createNestedObject("action");
  action["title"] = titles;
  action["status"] = statuss;
  action["msg"] = msgs;
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}
bool isLoggedin(IPAddress ip) {
  bool rt = false;
  for (int i = 0; i < UserLimit; i++) {
    if (Sessions[i].ipAddress == ip && Sessions[i].Expire == false && (millis() - Sessions[i].StartTime) <= TimeOut) {
      Sessions[i].StartTime = millis();
      rt = true;
    }
  }
  return rt;
}

bool LoginUser(IPAddress ip) {
  if (isLoggedin(ip)) {
    return true;
  }
  for (int i = 0; i < UserLimit; i++) {
    if (Sessions[i].Expire || ((millis() - Sessions[i].StartTime) > TimeOut)) {
      Sessions[i].Expire = false;
      Sessions[i].StartTime = millis();
      Sessions[i].ipAddress = ip;
      Serial.println("Access granted");
      return true;
    }
  }
  Serial.println("Limit reached. Access denied");
  PrintMessage("Limit reached. Access denied");
  return false;
}

void sendResponse(AsyncWebServerRequest* request, int statusCode, const char* contentType, const String& content) {
  request->send(statusCode, contentType, content);
}

//void readFileAndSendResponse(AsyncWebServerRequest* request, const char* filePath, const char* contentType, String str) {
//  HTTPClient http;
//  http.begin("http://192.168.43.2:5500"+String(filePath));
//  int httpResponseCode = http.GET();
//  if (httpResponseCode == HTTP_CODE_OK) {
//    String content = http.getString();
//    String contentType = "text/html"; // Replace with the appropriate content type
//    Serial.println("Loaded");
//    AsyncWebServerResponse *response = request->beginResponse(200, contentType, content);
//    response->addHeader("X-Identification", str);
//    request->send(response);
//  } else {
//    Serial.println("No");
//    sendResponse(request, 404, "text/plain", "File not found");
//  }
//}

void readFileAndSendResponse(AsyncWebServerRequest* request, const char* filePath, const char* contentType, String str) {
  File file = SPIFFS.open(filePath, "r");
  if (file) {
    String content = file.readString();
    file.close();
    AsyncWebServerResponse *response = request->beginResponse(200, contentType, content);
    if (content != "") {
      response->addHeader("X-Identification", str);
    }

    request->send(response);
  } else {
    sendResponse(request, 404, contentType, "File not found");
  }
}
void handleRoot(AsyncWebServerRequest* request) {
  //readFileAndSendResponse(request, "/index.html", "text/html", "index");
  request->send(SPIFFS, "/index.html", "text/html");
}
void handleScript(AsyncWebServerRequest* request) {
  request->send(SPIFFS, "/script.js", "text/javascript");
}
void handleSweet(AsyncWebServerRequest* request) {

  request->send(SPIFFS, "/sw_alert.js", "text/javascript");
}
void handleStyle(AsyncWebServerRequest* request) {
  request->send(SPIFFS, "/style.css", "text/css");
}
void handleRequest(AsyncWebServerRequest* request) {
  if (isLoggedin(request->client()->remoteIP())) {
    readFileAndSendResponse(request, "/Config.json", "application/json", "config");
  } else {
    readFileAndSendResponse(request, "/login.json", "application/json", "login");
    request->redirect("/redirect");
  }
}
void handleDefault(AsyncWebServerRequest* request) {
  if (isLoggedin(request->client()->remoteIP())) {
    readFileAndSendResponse(request, "/default.json", "application/json", "default");
  } else {
    request->redirect("/redirect");
  }
}
void handleSetting(AsyncWebServerRequest* request) {
  if (isLoggedin(request->client()->remoteIP())) {
    doActionAndSendResponse(request, "action", "application/json", "setting");
  } else {
    request->redirect("/redirect");
  }
}
void handleStoreRequest(AsyncWebServerRequest* request) {
  if (isLoggedin(request->client()->remoteIP())) {
    if(request->hasArg("storeid")){
      if (request->arg("storeid").equals("1")) {
        String postData = "token=" + Token + "&store=1";
        Serial.println(postData);
        String endpoint = "esp_stores";
        makeAsyncHttpPostRequest(endpoint, postData, request, [](JsonDocument doc, AsyncWebServerRequest* req) {
          String jsonString="";
          serializeJson(doc, jsonString);
          sendResponse(req, 200, "application/json", jsonString);
        });
      }else{
        String postData = "token=" + Token + "&store="+request->arg("storeid");
        String endpoint = "esp_stores";
        makeAsyncHttpPostRequest(endpoint, postData, request, [](JsonDocument doc, AsyncWebServerRequest* req) {
          String jsonString="";
          serializeJson(doc, jsonString);
          sendResponse(req, 200, "application/json", jsonString);
        });
      }
      
    }
  } else {
    request->redirect("/redirect");
  }
}
void handleLogin(AsyncWebServerRequest* request) {
  if (request->arg("user").equals(user) && request->arg("pass").equals(pass)) {
    IPAddress ipAddress = request->client()->remoteIP();
    if (LoginUser(ipAddress)) {
      request->redirect("/redirect");
    } else {
      sendResponse(request, 401, "text/html", "Limit reached");
    }

  } else {
    sendResponse(request, 401, "text/html", "Invalid credentials.");
  }
}
void handleLogout(AsyncWebServerRequest* request) {
  for (int i = 0; i < UserLimit; i++) {
    if (Sessions[i].ipAddress == request->client()->remoteIP()) {
      Serial.println("Found IP");
      Sessions[i].Expire = true;
      request->redirect("/redirect");
      break;
    }
  }
}

void handleAccess(AsyncWebServerRequest* request) {
  String postData = "email=" + request->arg("email") + "&password=" + request->arg("password");
  String endpoint = "esp_login";
  makeAsyncHttpPostRequest(endpoint, postData, request, [](JsonDocument doc, AsyncWebServerRequest* req) {
    msg = doc["msg"].as<String>();
    Token = doc["data"]["token"].as<String>();
    UserId = doc["data"]["userid"].as<String>();
    Access=true;
    req->redirect("/redirect");
  });
}

void handleConfig(AsyncWebServerRequest* request) {
  String postData = "deviceid=" + request->arg("devices") + "&token=" + Token + "&userid=" + UserId + "&mac=" + mac;
  Serial.println(postData);
  String endpoint = "esp_dev";
  makeAsyncHttpPostRequest(endpoint, postData, request, [](JsonDocument doc, AsyncWebServerRequest* req) {
    Serial.println(doc["msg"].as<String>());
    sendResponse(req, 200, "application/json", createJson("", doc["status"].as<String>(),  doc["msg"].as<String>()));
  });
}
void handleReset(AsyncWebServerRequest* request) {
  if (request->arg("user").equals(user) && request->arg("pass").equals(pass)) {
    if (request->arg("pass1").equals(request->arg("pass2"))) {
      Preferences preferences;
      preferences.begin("my-app", false);
      preferences.putString("user", request->arg("user"));
      preferences.putString("pass", request->arg("pass1"));
      preferences.end();
      handleLogout(request);
    } else {
      sendResponse(request, 200, "application/json", createJson("Invalid info", "error", "Password must match"));
    }
  } else {
    sendResponse(request, 200, "application/json", createJson("Invalid info", "error", "Invalid credentials"));
  }
}

void handleSubmit(AsyncWebServerRequest* request) {
  if (isLoggedin(request->client()->remoteIP())) {
    size_t paramsCount = request->params();
    Preferences preferences;
    preferences.begin("my-app", false);
    for (size_t i = 0; i < paramsCount; i++) {
      AsyncWebParameter* param = request->getParam(i);
      if (param->isPost()) {
        String paramName = param->name();
        String paramValue = param->value();
        preferences.putString(paramName.c_str(), paramValue);
      }
    }
    preferences.end();
    sendResponse(request, 200, "application/json", createJson("Save", "success", "Setting saved successfully"));
  } else {
    request->redirect("/redirect");
  }
}

void handleIP(AsyncWebServerRequest* request) {
  if (WiFi.getMode() == WIFI_AP) {
    IPAddress ip = WiFi.softAPIP();
    request->send(200, "text/plain", ip.toString());
  } else if (WiFi.getMode() == WIFI_STA) {
    IPAddress ip = WiFi.localIP();
    request->send(200, "text/plain", ip.toString());
  }
}



void handleOnlineUpdate(AsyncWebServerRequest* request) {
  if (isLoggedin(request->client()->remoteIP())) {
    if (CheckUpdates(false)) {
      UpdateFirmware = true;
      sendResponse(request, 200, "application/json", createJson("Updates", "success", "Syetem will upgrade to ver: " + String(FirmwareVersion)));
    } else {
      sendResponse(request, 200, "application/json", createJson("Failed", "error", "No new update available"));
    }
  } else {
    request->redirect("/redirect");
  }
}
void handleUpdateFromLink(AsyncWebServerRequest* request) {
  if (isLoggedin(request->client()->remoteIP())) {
    InitOTA(request->arg("link"));
    if (CheckUpdates(false)) {
      UpdateFirmware = true;
      sendResponse(request, 200, "application/json", createJson("Updates", "success", "Syetem will upgrate to ver: " + String(FirmwareVersion)));
    } else {
      sendResponse(request, 200, "application/json", createJson("Failed", "error", "No new update available"));
    }
  } else {
    request->redirect("/redirect");
  }
}
void handleReboot(AsyncWebServerRequest* request) {
  if (isLoggedin(request->client()->remoteIP())) {
    Serial.println("Rebooting...");
    PrintMessage("Rebooting...");
    request->redirect("/redirect");
    ESP.restart();
  } else {
    request->redirect("/redirect");
  }
}




void uploadFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    request->_tempFile = SPIFFS.open("/" + filename, "w");
  }
  if (len) {
    request->_tempFile.write(data, len);
  }
  if (final) {
    request->_tempFile.close();
    sendResponse(request, 200, "application/json", createJson("File upload", "success", "File uploaded successfully."));
  }
}
void handleFileDelete(AsyncWebServerRequest *request) {
  if (request->hasParam("file", true)) {
    AsyncWebParameter *fileParam = request->getParam("file", true);
    String fileName = fileParam->value();
    Serial.println("File name received: " + fileName);
    SPIFFS.remove("/" + fileName);
    sendResponse(request, 200, "application/json", createJson("File Deleted", "success", "File deleted successfully."));
  } else {
    sendResponse(request, 200, "application/json", createJson("No file", "warning", "No file selected to delete"));
  }
}
void WebServices() {

  server.on("/", HTTP_GET, handleRoot);
  server.on("/rediect", HTTP_GET, handleRoot);
  server.on("/req", HTTP_GET, handleRequest);
  server.on("/def", HTTP_GET, handleDefault);
  server.on("/set", HTTP_GET, handleSetting);
  server.on("/sw_alert.js", HTTP_GET, handleSweet);
  server.on("/script.js", HTTP_GET, handleScript);
  server.on("/style.css", HTTP_GET, handleStyle);
  server.on("/access", HTTP_POST, handleAccess);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/logout", HTTP_POST, handleLogout);
  server.on("/config", HTTP_POST, handleConfig);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.on("/reset", HTTP_POST, handleReset);
  server.on("/ip", HTTP_GET, handleIP);
  server.on("/rst", HTTP_POST, handleReboot);
  server.on("/onlinefirmware", HTTP_POST, handleOnlineUpdate);
  server.on("/linkfirmware", HTTP_POST, handleUpdateFromLink);
  server.on("/delete", HTTP_POST, handleFileDelete);
  server.on("/store", HTTP_POST, handleStoreRequest);
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
      File file = SPIFFS.open("/favicon.ico", "r");
      request->send(file, "image/x-icon");
      file.close();
    });
  server.on("/uploadfile", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    request->send(200);
  }, uploadFile);
  server.on("/uploadfirmware", HTTP_POST, [](AsyncWebServerRequest * request) {
    int fileCount = request->params();
    bool exist = false;
    for (int i = 0; i < fileCount; i++) {
      AsyncWebParameter *param = request->getParam(i);
      if (param->isFile()) {
        exist = true;
        break;
      }
    }
    if (exist) {
      rebooting = !Update.hasError();
      if (rebooting) {
        sendResponse(request, 200, "application/json", createJson("Firmware updated", "success", "Firmware updated successfully. your system rebooting now..."));
      } else {
        sendResponse(request, 200, "application/json", createJson("Update failed", "error", "Firmware update failed, Try again"));
      }
      if (rebooting) {
        delay(100);
        ESP.restart();
      }
    } else {
      sendResponse(request, 200, "application/json", createJson("No Firmware", "error", "No firmware selected. Please select firmware first"));
    }

  },
  [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      Serial.print("Updating: ");
      PrintMessage("Updating: ");
      Serial.println(filename.c_str());
      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
        Update.printError(Serial);
      }
    }
    if (!Update.hasError()) {
      if (Update.write(data, len) != len) {
        Update.printError(Serial);
      }
    }
    if (final) {
      if (Update.end(true)) {
        Serial.print("The update is finished: ");
        Serial.println(convertFileSize(index + len));
        PrintMessage("The update is finished: " + convertFileSize(index + len));
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.begin();
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }
}

#endif
