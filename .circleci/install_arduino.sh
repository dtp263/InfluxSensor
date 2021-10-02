#!/usr/bin/env bash

sudo apt-get install bzip2
yes | sudo apt install python-pip
pip install pyserial
pip install --upgrade pip

wget -O arduino-cli-linux64 https://github.com/arduino/arduino-cli/releases/download/0.19.2/arduino-cli_0.19.2_Linux_64bit.tar.gz | tar -xz

sudo mv arduino-cli-linux64 /usr/local/share/arduino-cli
sudo ln -s /usr/local/share/arduino-cli /usr/local/bin/arduino-cli

printf "board_manager:
  additional_urls:
    - https://dl.espressif.com/dl/package_esp32_index.json
    - https://arduino.esp8266.com/stable/package_esp8266com_index.json" >> .cli-config.yml
sudo mv .cli-config.yml /usr/local/share/

arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli core install esp8266:esp8266

arduino-cli lib install ArduinoJson
arduino-cli lib install WiFiManager
arduino-cli lib install "ESP8266 Influxdb"
