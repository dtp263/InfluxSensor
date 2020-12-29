// #include <ArduinoOTA.h>
// #include <WiFiUdp.h>
// #include <WiFiManager.h>
// #include <DNSServer.h>
// #include <ESP8266mDNS.h>
// #include <ESP8266WiFi.h>
// #include <string.h>

// class OTA
// {
// private:
//   uint32_t _ESP_ID;

// public:
//   OTA()
//   {
//     Serial.begin(115200);
//     this->_ESP_ID = ESP.getChipId();
//     Serial.println("This is where we're at:");
//     Serial.println("ESP8266 ID:");
//     Serial.println(String(this->_ESP_ID));
//   }

//   void setup_ota(const char* ssid, const char* password)
//   {
//     Serial.println("Booting");
//     WiFi.mode(WIFI_STA);
//     wifiMulti.addAP(ssid, password);

//     Serial.print("Connecting to wifi");
//     while (wifiMulti.run() != WL_CONNECTED) {
//       Serial.print(".");
//       delay(100);
//     }
//     Serial.println();

//     ArduinoOTA.onStart([]() {
//       String type;
//       if (ArduinoOTA.getCommand() == U_FLASH) {
//         type = "sketch";
//       } else { // U_FS
//         type = "filesystem";
//       }

//       // NOTE: if updating FS this would be the place to unmount FS using FS.end()
//       Serial.println("Start updating " + type);
//     });
//     ArduinoOTA.onEnd([]() {
//       Serial.println("\nEnd");
//     });
//     ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//       Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
//     });
//     ArduinoOTA.onError([](ota_error_t error) {
//       Serial.printf("Error[%u]: ", error);
//       if (error == OTA_AUTH_ERROR) {
//         Serial.println("Auth Failed");
//       } else if (error == OTA_BEGIN_ERROR) {
//         Serial.println("Begin Failed");
//       } else if (error == OTA_CONNECT_ERROR) {
//         Serial.println("Connect Failed");
//       } else if (error == OTA_RECEIVE_ERROR) {
//         Serial.println("Receive Failed");
//       } else if (error == OTA_END_ERROR) {
//         Serial.println("End Failed");
//       }
//     });
//     ArduinoOTA.begin();
//     Serial.println("Ready");
//     Serial.print("IP address: ");
//     Serial.println(WiFi.localIP());
//   }

//   void handle_ota()
//   {
//     Serial.println("Begin Over the Air Update:");
//     ArduinoOTA.handle();
//     Serial.println("OTA Update Complete.");
//   }
// };
