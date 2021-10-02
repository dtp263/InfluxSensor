#!/usr/bin/env bash

arduino-cli compile --output temp.bin -b esp8266:esp8266:nodemcuv2 $PWD/InfluxSensor.ino --debug