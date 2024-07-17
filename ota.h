#include <esp32fota.h>

esp32FOTA esp32FOTA("Stores",FIRMWARE_VERSION , false);


void InitOTA(String url=manifest_url){
  Serial.print(manifest_url);
  esp32FOTA.setManifestURL(url);
  esp32FOTA.useDeviceId( true );
  esp32FOTA.printConfig();
}

bool CheckUpdates(bool _update){
  bool updatedNeeded = esp32FOTA.execHTTPcheck();
  
  char payloadVersion[16]="";
  esp32FOTA.getPayloadVersion(payloadVersion);
  if(updatedNeeded){
    Serial.print("New Update Available V: ");
    FirmwareVersion=String(payloadVersion);
    Serial.println(FirmwareVersion);
  }
  if(_update){
    if (updatedNeeded){
      PrintMessage("Firmware updating from server...");
      Serial.println(esp32FOTA.execOTA());
      return true;
    }else{
      return false;
    }
  }else{
    return updatedNeeded;
  }
}
