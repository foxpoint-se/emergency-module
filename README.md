# emergency-module
C-code (Arduino) for the emergency module, responsible for detecting when Alen has been under water for to long.

## Functionality
Purpose of the emergency module is to detect if the Alen AUV has been under water for to long (15 mins) indicating that it has run into problems. When this happens a servo will move triggering a release mechanism that detaches the emergency module from the AUV. 

When the AUV and the emergency module is in surface mode, the emergency module waits for a GPS fix and transmits its position on a given frequency. 

There is also a current sensor connected to the emergency module to be able to determine for how much longer the emergency module can operate.

## Components
Emergency module is built on the following components.

- Arduino MKR WAN 1310
- Ublox NEO-6M GPS Module
- Adafruit INA219 Current Sensor
- Servo HighTech HS-40

## Dependencies & installation
There are two external dependencies to other Arduino libraries. One for the LoRa tranciever and one for the adafruit Ina219 current sensor.

LoRa module can be installed from this Git repository:
https://github.com/sandeepmistry/arduino-LoRa

INA219 Library:
https://github.com/adafruit/Adafruit_INA219 (Also available on official Arduino lib manager)

Also the official board software needs to be installed from the Arduino IDE.

## Data format
Emergency module transmits an aggregated data message separated by a newline character in the following format:

221835.00\n1\n17.981023\n59.307035\n35.00\n

1. UTC time parsed from the GPS module, ex: 221835.00
2. Status (0 - Normal operation, 1 - Emergency mode), ex: 1
3. Longitude parsed from the GPS module, ex: 17.981023
4. Latitude parsed form the GPS module, ex: 59.307035
5. Battery percentage, current votlage from INA sensor over max battery voltage, ex: 35.00