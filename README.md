# esp-solar-contoller

esp8266 and esp32 based solar controllers targetting Houses with a fronius solar system (other inverters can be supported).

Inspired by the various DIY powerwalls

https://www.facebook.com/groups/diypowerwalls/

## Use

Common Day devices:

* Pool Pump
* powerwall / UPS / Battery Bank charger
* roof exhuast fan

Common Night time Devices:

* UPS mains disconnect
* Grid tie inverter turn on

## Example Applications

**Simple**

(day) Turn on day time device when PV export is greater than device size (eg pool pump)


**DIY Powerwall**

(Day) Turn on charger when PV export is greater than charger size.

(night) Turn on inverter when power import is larger than inverter size.

**UPS**

(Day) Turn on charger when PV export is greater than charger size.

(night) disconnect UPS mains if house is drawing power from the grid.

## Features

* Self hosted solution, no external servers needed everything runs on the microchip.
* Complete Web Interface for confguration and monitoring.
* JSON config file for easy manual editing.
* OLED display
* Support querying Fronius inverters directly
* NTP time sync
* Fallback WiFi network settings.

### Battery Monitoring (esp32)

* High voltage disconect (day)
* Low Cell Voltage disconnect (night)
* High temp shutdown

## esp8266

Supporrt for all common esp8266 boards.

* sonoff
* IoT Yunshun
* Anything tasmota runs on.

Supports adding OLED display the above.

## ESP32 Board

Custom designed board.

* 16 Bit 4 channel ADC fed to 4 8x analog signal multiplexers allowing for a total of 32 analog lines.

* 16 Voltage monitoring lines.
* 16 ntc temp sensors + 1 ambient temp sensor.
* SD Card for config storage.
* Simple Oled display.
* Piezo buzzer.
* 5v & 3.3v i2c headers for expansion.
