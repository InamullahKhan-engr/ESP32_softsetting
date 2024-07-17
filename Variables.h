#ifndef VARIABLES_H
#define VARIABLES_H
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#define TimeOut 600000
#define UserLimit 3   

#define AntennaRed 34
#define AntennaGreen 5//35
#define Relay 25 //23
#define Connection 2
#define Enable_pin 33 
//#define Relay_pin 25 //23
//#define Signal2 34
int state = 0, prestate = 0;
#define FIRMWARE_VERSION "0.0.45"
extern boolean WSConn = false;
extern boolean Enable = false;

String manifest_url="";
extern int port = 3000;
extern boolean Access = false;
extern String Token = "";
extern String UserId = "";

extern String msg = "Enter your website email and password to access the server.";
extern String mac = "";
extern String host = "magnifo";
extern String passap = "12345678";
extern String ssidsta = "YourWifi";
extern String passsta = "password";
extern int server_ip0=192, server_ip1=168, server_ip2=43, server_ip3=219;
extern int ap_ip0=192, ap_ip1=168, ap_ip2=13, ap_ip3=14;
extern String user="admin";
extern String pass="admin";
extern int ap_idle_tme=60;
IPAddress local_IP,server_IP;
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);
bool rebooting = false;
bool UpdateFirmware=false;
String FirmwareVersion="";
long wifilost;
struct Session {
  unsigned long StartTime;
  IPAddress ipAddress;
  bool Expire=true;
};
Session Sessions[UserLimit];

void InitPref(){
  Preferences preferences;
  preferences.begin("my-app", false);
  host=preferences.getString("host", host);
  passap = preferences.getString("passap", passap);
  ssidsta = preferences.getString("ssidsta", ssidsta);
  passsta = preferences.getString("passsta", passsta);
  server_ip0 = preferences.getString("server_ip0", String(server_ip0)).toInt();
  server_ip1 = preferences.getString("server_ip1", String(server_ip1)).toInt();
  server_ip2 = preferences.getString("server_ip2", String(server_ip2)).toInt();
  server_ip3 = preferences.getString("server_ip3", String(server_ip3)).toInt();
  
  ap_ip0 = preferences.getString("ap_ip0", String(ap_ip0)).toInt();
  ap_ip1 = preferences.getString("ap_ip1", String(ap_ip1)).toInt();
  ap_ip2 = preferences.getString("ap_ip2", String(ap_ip2)).toInt();
  ap_ip3 = preferences.getString("ap_ip3", String(ap_ip3)).toInt();
  user = preferences.getString("user", user);
  pass = preferences.getString("pass", pass);

  port = preferences.getString("port", String(port)).toInt();
  ap_idle_tme = preferences.getString("ap_idle_tme", String(ap_idle_tme)).toInt();
  local_IP = IPAddress(ap_ip0, ap_ip1, ap_ip2, ap_ip3);
  server_IP = IPAddress(server_ip0, server_ip1, server_ip2, server_ip3);
  preferences.end();
}
void PrintMessage(String str){
  //ws.textAll(str);
  Serial.println(str);
}
#endif
