# Arduino PMC Clock Library

This repo is no depreciated due to the new [https://github.com/arduino-libraries/Arduino_PortentaMachineControl](Arduino Portenta Machine Control Library).

The Arduino PMC Clock Library provides an easy way to get milisecond unix epoch timestamps, synchronizing the millis() function on your Arduino Portenta Machine Control board with a Network Time Protocol (NTP) server and the Real Time Clock (RTC). This library helps to keep accurate time on your board by automatically synchronizing with an NTP server and the RTC periodically to minimize drift. The ArduinoPMCClock class is based on NTPClient examples and Arduino Portenta Machine Control Real Time Clock examples.

- Works with the Arduino Portenta Machine control, will be modified/tested to also work with the Arduino Opta at some point.
- Minimal testing has been done.

## Features
- Synchronize Arduino millis() with an NTP server
- Periodically sync to maintain accurate time
- Automatically handle millis() rollover
- Customizable NTP and RTC sync intervals
- Supports multiple constructors for different configurations
- Easily get a unix epoch timestamp in milliseconds (uint64_t) with the getEpochMillis method
- Uses millis() function to get timestamp, and syncs that with the RTC at a fixed rate, reducing i2c throughput on RTC 

## Installation
- Make sure you have the NTPClient, Arduino_MachineControl, and Udp libraries installed.
- Download the source files ArduinoPMCClock.h and ArduinoPMCClock.cpp as a ZIP.
- In the Arduino IDE, navigate to Sketch > Include Library > Add .ZIP Library.
- Select the downloaded ZIP file to install the library.

## Usage
Include the library and create an instance of the ArduinoClock class by passing a reference to a UDP instance:
```cpp
#include <ArduinoPMCClock.h>
#include <Arduino_ConnectionHandler.h>

EthernetUDP ntpUDP; // Can be WiFiUDP or EthernetUDP
ArduinoClock arduino_clock(ntpUDP);

```
Begin the NTP client upon network connection:
```cpp
void onNetworkConnect() {
    arduino_clock.ntpClientBegin();
}
```
End the NTP client when network connection is lost (The begin and end work well with the Arduino_ConnectionHandler library):
```cpp
void onNetworkDisconnect() {
    arduino_clock.ntpClientEnd();
}
```
Call the tick() method in the loop, to keep synchronizing according to the NTP Sync and RTC Sync rates:
```cpp
void loop() {
  arduino_clock.tick();
}
```
Get the current Unix timestamp in milliseconds using getEpochMillis():
```cpp
uint64_t currentTimestamp = arduino_clock.getEpochMillis();
```

## Example
Here's a complete example that demonstrates how to use the Arduino PMC Clock Library:
```cpp
#include <Arduino_ConnectionHandler.h>
#include <ArduinoPMCClock.h>

EthernetConnectionHandler connection_handler;
EthernetUDP ntpUDP;

// Replace with your local NTP server address
const char* timeServer = "pool.ntp.org";

ArduinoClock arduino_clock(ntpUDP, timeServer);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  connection_handler.addCallback(NetworkConnectionEvent::CONNECTED, onNetworkConnect);
  connection_handler.addCallback(NetworkConnectionEvent::DISCONNECTED, onNetworkDisconnect);
}

void loop() {

  // Check network connection before continuing, put all network dependant functions behind this block
  NetworkConnectionState currentState = connection_handler.check();
  if (currentState != NetworkConnectionState::CONNECTED) {
      return;
  }

  arduino_clock.tick();

  uint64_t currentTimestamp = arduino_clock.getEpochMillis();
  Serial.print("Current Timestamp: ");
  Serial.println(currentTimestamp);

  delay(1000);
}

void onNetworkConnect() {
    arduino_clock.ntpClientBegin();
    arduino_clock.forceSyncNTP(); // Sync on connect
}

void onNetworkDisconnect() {
    arduino_clock.ntpClientEnd();
}
```
