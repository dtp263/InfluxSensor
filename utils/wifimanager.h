#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "wifiparam.h"

WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputState = "off";

// Assign output variables to GPIO pins
char output[2] = "5";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

class CustomWifiManager
{
private:
  uint32_t _ESP_ID;
  char *sensorLabel;
  WiFiManager wifiManager;
  std::vector<WifiParam> params;

  // DynamicJsonDocument configJson;

public:
  char *dbBucket;

  CustomWifiManager()
  {
    Serial.begin(115200);
    this->_ESP_ID = ESP.getChipId();
    Serial.println("This is where we're at:");
    Serial.println("ESP8266 ID:");
    Serial.println(String(this->_ESP_ID));
    this->sensorLabel = "sensor-1";
    this->dbBucket = "default";
    // DynamicJson doc(1024);
    // this->configJson = doc;
  }

  // @id is used for HTTP queries and must not contain spaces nor other special characters
  void add_param(char *id, char *placeholder, char *defaultValue, int length)
  {
    params.push_back(WifiParam(id, placeholder, defaultValue, length));
  }

  char *getParamValue(char *id)
  {
    for (int i; i < this->params.size(); i++) {
      if (this->params[i].id == id) {
        return (char*)this->params[i].getValue();
      }
    }
  }

  void apply_wfm_options()
  {
    // WiFiManager
    // Local intialization. Once its business is done, there is no need to keep it around
    // WiFiManager wifiManager;

    //set config save notify callback
    this->wifiManager.setSaveConfigCallback(saveConfigCallback);

    // set custom ip for portal
    //this->wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    // Uncomment and run it once, if you want to erase all the stored information
    this->wifiManager.resetSettings();

    //set minimu quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //this->wifiManagerwifiManager.setMinimumSignalQuality();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    this->wifiManager.setTimeout(1200);

    // fetches ssid and pass from eeprom and tries to connect
    // if it does not connect it starts an access point with the specified name
    // here  "AutoConnectAP"
    // and goes into a blocking loop awaiting configuration
    // this->wifiManager.autoConnect("AutoConnectAP");
    // or use this for auto generated name ESP + ChipID
    this->wifiManager.autoConnect();
  }

  void load_from_config()
  {
    Serial.println("mounting FS...");
    if (SPIFFS.begin())
    {
      Serial.println("mounted file system");
      if (SPIFFS.exists("/config.json"))
      {
        //file exists, reading and loading
        Serial.println("reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile)
        {
          Serial.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);
          // DynamicJsonBuffer jsonBuffer;
          DynamicJsonDocument doc(1024);
          // JsonObject &json = jsonBuffer.parseObject(buf.get());
          deserializeJson(doc, buf.get());

          JsonObject obj = doc.as<JsonObject>();

          Serial.println(obj.size());

          for (int i; i < this->params.size(); i++)
          {
            Serial.println("Check if saved value exists for");
            Serial.println(this->params[i].id);
            if (obj.containsKey(this->params[i].id))
            {
              strcpy(this->params[i].defaultValue, obj[this->params[i].id]);
            }
          }
        }
      }
    }
    else
    {
      Serial.println("failed to mount FS");
    }
  }

  void setup_wifi_manager()
  {
    Serial.println("Start setup.");
    // Serial.begin(115200);

    //clean FS, for testing
    //SPIFFS.format();

    //read configuration from FS json
    this->load_from_config();
    //end read

    for (int i; i < this->params.size(); i++)
    {
      Serial.println("Add param to wifiManager");
      Serial.println(this->params[i].id);
      this->params[i].createParam();
      this->wifiManager.addParameter(this->params[i].param);
    }

    this->apply_wfm_options();

    // if you get here you have connected to the WiFi
    Serial.println("Connected.");

    //save the custom parameters to FS
    if (shouldSaveConfig)
    {
      Serial.println("saving config");
      DynamicJsonDocument doc(1024);

      for (int i; i < this->params.size(); i++)
      {
        doc[this->params[i].id] = (char*)this->params[i].getValue();
      }

      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile)
      {
        Serial.println("failed to open config file for writing");
      }

      Serial.println("file open prepare to write");

      serializeJsonPretty(doc, Serial);
      serializeJsonPretty(doc, configFile);

      configFile.close();

      Serial.println("config save complete.");
      //end save
    }

    server.begin();
    Serial.println("End setup.");
  }

  void do_loop()
  {
    Serial.println("Do WFM loop.");
    WiFiClient client = server.available(); // Listen for incoming clients

    if (client)
    {                                // If a new client connects,
      Serial.println("New Client."); // print a message out in the serial port
      String currentLine = "";       // make a String to hold incoming data from the client
      while (client.connected())
      { // loop while the client's connected
        if (client.available())
        {                         // if there's bytes to read from the client,
          char c = client.read(); // read a byte, then
          Serial.write(c);        // print it out the serial monitor
          header += c;
          if (c == '\n')
          { // if the byte is a newline character
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0)
            {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              // turns the GPIOs on and off
              if (header.indexOf("GET /output/on") >= 0)
              {
                Serial.println("Output on");
                outputState = "on";
                // digitalWrite(atoi(output), HIGH);
              }
              else if (header.indexOf("GET /output/off") >= 0)
              {
                Serial.println("Output off");
                outputState = "off";
                // digitalWrite(atoi(output), LOW);
              }

              // Display the HTML web page
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              // CSS to style the on/off buttons
              // Feel free to change the background-color and font-size attributes to fit your preferences
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
              client.println(".button2 {background-color: #77878A;}</style></head>");

              // Web Page Heading
              client.println("<body><h1>ESP8266 Web Server</h1>");

              // Display current state, and ON/OFF buttons for the defined GPIO
              client.println("<p>Output - State " + outputState + "</p>");
              // If the outputState is off, it displays the ON button
              if (outputState == "off")
              {
                client.println("<p><a href=\"/output/on\"><button class=\"button\">ON</button></a></p>");
              }
              else
              {
                client.println("<p><a href=\"/output/off\"><button class=\"button button2\">OFF</button></a></p>");
              }
              client.println("</body></html>");

              // The HTTP response ends with another blank line
              client.println();
              // Break out of the while loop
              break;
            }
            else
            { // if you got a newline, then clear currentLine
              currentLine = "";
            }
          }
          else if (c != '\r')
          {                   // if you got anything else but a carriage return character,
            currentLine += c; // add it to the end of the currentLine
          }
        }
      }
      // Clear the header variable
      header = "";
      // Close the connection
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
    }
  }

  char *getSensorLabel()
  {
    return this->sensorLabel;
  }
};