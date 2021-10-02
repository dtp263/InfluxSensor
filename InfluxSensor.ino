#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <string.h>
#include "utils/influx.h"
#include "utils/ota.h"
#include "utils/wifimanager.h"
#include <ArduinoOTA.h>
#include <vector>
#include "../secrets_example.h"

using std::vector;

// OTA ota;
CustomWifiManager wfm;
Influx influx;

// const char *ssid = STASSID;
// const char *password = STAPSK;

const int AirValue = 750; //you need to replace this value with Value_1
const int WaterValue = 325;

int getSoilMoistureValue()
{
  return map(analogRead(A0), WaterValue, AirValue, 0, 100);
}

void setup()
{
  // Add params to wifiManager.
  wfm.add_param("node_id", "Node ID", "basil-1", 60);
  wfm.add_param("influx_bucket", "Influx Bucket", "bucket-2", 60);
  wfm.add_param("influx_token", "Influx Token", "S8u.....", 89);
  wfm.add_param("influx_url", "Influx Url", "https://us-central1-1.gcp.cloud2.influxdata.com", 89);
  wfm.add_param("influx_org", "Influx Org", "olin.davis20@gmail.com", 60);

  wfm.setup_wifi_manager();

  // Add sensors to Influx
  Sensor soilMoisture("soil_moisture", "wetness", getSoilMoistureValue);

  Serial.println("NODE_ID");
  Serial.println(wfm.getParamValue("node_id"));

  // Add tags
  soilMoisture.addTag("node_id", (String)wfm.getParamValue("node_id"));

  influx.add_sensor(soilMoisture);

  // ota.setup_ota(ssid, password);

  influx.setup_influx("ESP8266");
}

void loop()
{
  // ota.handle_ota();
  wfm.do_loop();

  influx.run_influx();
  delay(3000);
}
