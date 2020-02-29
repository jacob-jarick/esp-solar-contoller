# esp-solar-contoller
 esp8266 and esp32 based solar controllers targetting fronius inverter users

## Use

Houses with solar.

I use this device to monitor my Fronius Inverters. It queries the JSON API and finds out how much power the house is exporting or importing.

Two loads are supported per controller, one for day and one for night.

additionally the device can be configured as a timer or a button bassed timer.

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

### Day (charger)

If solar export is greater than configured charger size (watts) turn on charger.

Common Day devices:

* Pool Pump
* powerwall / UPS / Battery Bank charger
* roof exhuast fan

### Night (Drain)

When the house power import is greater than the night device size (watts) turn on the device.

Common Night time Devices:

* UPS mains disconnect
* Grid tie inverter turn on

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
