#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <string.h>
#include "sensor.h"
#include <ESP8266WiFiMulti.h>
#include "../secrets_example.h"

#define DEVICE "ESP8266"
// // InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
// #define INFLUXDB_URL "https://us-central1-1.gcp.cloud2.influxdata.com"
// // InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
// #define INFLUXDB_TOKEN "S8uyv99fnXFdNlt0JaX6IiECKjydVKQS-hawqDDBtG-pjYXOzvYtAk0kg66EsRAgpAOJ95jG2I6KPd_G9bpMuw=="
// // InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
// #define INFLUXDB_ORG "olin.davis20@gmail.com"
// // InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
// #define INFLUXDB_BUCKET "esp8266bucket"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "EST5EDT"

// InfluxDB client instance with preconfigured InfluxCloud certificate
// InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
InfluxDBClient client;
ESP8266WiFiMulti wifiMulti;

// Data point
Point sensor("wifi_status");

class Influx
{
private:
  InfluxDBClient _influxClient;
  std::vector<Sensor> _sensors;

public:
  Influx()
  {
    Serial.println("Influx object created:");
    // this->_influxClient = client;
    client.setConnectionParams(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  }

  void setup_influx(String device)
  {
    for (int i; i < _sensors.size(); i++)
    {
      _sensors[i].addTag("device", device);
    }

    // Add tags
    sensor.addTag("device", device);
    sensor.addTag("SSID", WiFi.SSID());

    // Accurate time is necessary for certificate validation and writing in batches
    // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
    // Syncing progress and the time will be printed to Serial.
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

    // Check server connection
    if (client.validateConnection())
    {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    }
    else
    {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }

    // WriteOptions options;

    // this is +1 because we are adding the wifi strength in after the fact atm.
    // client.setWriteOptions(options.batchSize(3));
  }

  void add_sensor(Sensor s)
  {
    _sensors.push_back(s);
  }

  void run_influx()
  {
    // Clear fields for reusing the point. Tags will remain untouched
    sensor.clearFields();

    for (int i; i < _sensors.size(); i++)
    {
      _sensors[i].clearFields();
    }

    // Store measured value into point
    // Report RSSI of currently connected network
    sensor.addField("rssi", WiFi.RSSI());

    for (int i; i < _sensors.size(); i++)
    {
      _sensors[i].addField(_sensors[i].getUnit(), _sensors[i].getValue());
    }

    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(sensor.toLineProtocol());

    for (int i; i < _sensors.size(); i++)
    {
      Serial.print("Writing: ");
      Serial.println(_sensors[i].getPoint().toLineProtocol());
    }

    // If no Wifi signal, try to reconnect it
    if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED))
    {
      Serial.println("Wifi connection lost");
    }

    for (int i; i < _sensors.size(); i++)
    {
      client.writePoint(_sensors[i].getPointRef());
    }

    // Write point
    if (!client.writePoint(sensor))
    {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }

    //Wait 10s
    Serial.println("Wait 1s");
    delay(1000);
  }
};