# Arduino Doorbell
## Author
👤 Owen Cook  
👤 Hayden Bischoff

## About
This project is a prototype smart doorbell system made with an _Arduino Uno R3_ and various sensors from the _ELEGOO Most Complete Starter Kit_. This project is being made for a final project for one of my classes here at UVM.

## Setup
1. Setup circuit as shown in **INSERT LINK TO CIRCUIT DIAGRAM**
2. Download [BLE Serial tiny](https://apps.apple.com/us/app/ble-serial-tiny/id1607862132)

> **_Optional:_** Run the code in **INSERT LINK TO FILE HERE**, following the comments to rename your HM-10 Bluetooth Device

## Parts List
|        Part       | Pins Used |
| ----------------- | --------- |
| Relay Module      | 1 |
| Push Button       | 2 |
| HM-10 Bluetooth   | 3, 4 |
| Ultrasonic Sensor | 5, 8 |
| 8x8 LED Matrix    | 6, 7, SCL |
| RFID Module       | 9, 10, 11, 12, 13 |
| Water Sensor      | A0 |
| 9G Mini Servo     | SDA |
| Arduino Uno R3    | N/A |
| Active Buzzer     | N/A |
| Power Module      | N/A |

## Libraries Used  
- OneWire by Jim Studt, Tom Pollard, and Robin James
- LedControl by Eberhard Fahle
- MRC522 by GitHub Community
- SoftwareSerial by Arduino
- Servo by Arduino
- EEPROM by Arduino

## Known Bugs
- Servo seems to activate randomly at times
- Serial message written in setup is buggy when sent to phone

