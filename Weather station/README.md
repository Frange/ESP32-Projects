# Foobar

This project consists of two ESP32 devices, the master is a weather station and a client to collect the information obtained.

The weather station has the following components:
- ESP32.
- 18650 battery.
- Solar panel.
- TP4056 (voltage regulator/charger).
- MH-DL18S and push button (check the battery by pressing the button).
- Rain sensor (Raindrops module).
- DHT22 (Temperature and humidity detector).
- Low dropout regulator (LDO) MCP1700-3302E.
- 100uF electrolytic capacitor.
- 100nF ceramic capacitor.
- 100k Ohm resistor.
- 27k Ohm resistor.

The client has the following components:
- ESP32.
- Buzzer (if you want to play sounds when it rains).
- 4 digit display.
