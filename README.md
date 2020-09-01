# Arrowhead Conveyor belt

This repository contains an Arrowhead Adapter program that encapsulates the neccessary services to operate with a basic conveyor belt. The service implementations presented here are based on the ArrowheadESP library and the Arrowhead Adapter codebase. (These are Arduino programs that are written primarily for the ESP8266 microcontroller.)

## Requirements

In order to compile this program (e.g. in Arduino IDE), you will need the following libraries:

* [ESP8266 Arduino Core](https://github.com/esp8266/Arduino) or [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32) if you are compiling to ESP microcontrollers
* [ArrowheadESP](https://github.com/arrowhead-f/ArrowheadESP)
* [Arrowhead Adapter](https://github.com/arrowhead-f/adapter-quick-demo-dev)

## Arrowhead configuration

In order to set up the Arrowhead adapter for your conveyor belt please refer to the documentation found in [this repository](https://github.com/arrowhead-f/adapter-quick-demo-dev)

## Services

### *transport-with-conveyor*

Moves the conveyor belt from its current position to a given distance. This service accepts a `signed-distance` parameter (as a GET parameter of its REST endpoint) that specifies the aforementioned distance in millimeters along with the direction of movement. A positive value means a forward direction (when the belt is moving away from the motor). A negative value means a backwards direction (when the belt is moving towards the motor).

### *transport-with-conveyor-end-to-end*

Moves the belt with an entire length of the conveyor. This service accepts a `direction` parameter (as a GET parameter of its REST endpoint) that specifies the direction of the movement. The value of this parameter can be either `forward` or `backwards`. (The meanings of these directions are mentioned above.)

## Notes

* Don't forget to modify the `shaftRadius` variable with your conveyor shaft radius in order to accurately calculate the movement distances.
