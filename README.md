# Arduino Doorbell
## Authors
👤 Owen Cook  
👤 Hayden Bischoff

## About
This project is a prototype smart doorbell system made with an _Arduino Uno R3_ and various sensors from the _ELEGOO Most Complete Starter Kit_. This project is being made for a final project for one of my classes here at UVM.

## Setup
1. Setup circuit as shown in [schematic](https://github.com/owncook/Arduino-Doorbell/blob/main/schematic.png)
2. Download [BLE Serial tiny](https://apps.apple.com/us/app/ble-serial-tiny/id1607862132)
3. When the circuit is connected, run the code in [get_rfid_id](https://github.com/owncook/Arduino-Doorbell/blob/main/get_rfid_id/get_rfid_id.ino) and copy and paste the tag ID into the acceptedTag variable on line 109 in [arduino_doorbell](https://github.com/owncook/Arduino-Doorbell/blob/7d867f3024a8e6fcdeb75cd9113df0d4b9efdf85/arduino_doorbell/arduino_doorbell.ino#L109) 

> **_Optional:_** Run the code in [rename_bluetooth](https://github.com/owncook/Arduino-Doorbell/blob/main/rename_bluetooth/rename_bluetooth.ino), following the comments to rename your HM-10 Bluetooth Device

## Parts List
|        Part       | Pins Used |
| ----------------- | --------- |
| Ultrasonic Sensor | 0, 1 |
| Relay Module      | 2 |
| HM-10 Bluetooth   | 3, 4 |
| Passive Buzzer    | 5 |
| 8x8 LED Matrix    | 6, 7, SCL |
| Push Button       | 8 |
| RFID Module       | 9, 10, 11, 12, 13 |
| Water Sensor      | A0 |
| Potentiometer     | A1 |
| 9G Mini Servo     | SDA |
| Arduino Uno R3    | N/A |
| Power Module      | N/A |

## Libraries Used  
- OneWire by Jim Studt, Tom Pollard, and Robin James
- LedControl by Eberhard Fahle
- MRC522 by GitHub Community
- SoftwareSerial by Arduino
- Servo by Arduino
- EEPROM by Arduino

## Project Requirements

### A: Problem and Story
| Requirement | Fulfillment |
|-------------|-------------|
| What problem do you plan to solve? What is your story? | Transform modern doorbell design to be more useful and convenient |

### B: Inputs
| Requirement | Fulfillment |
|-------------|-------------|
| At least one digital sensor that we used in lab | Push Button |
| One digital sensor that we have not used yet in lab | RFID Sensor |
| At least one Analog input | Potentiometer |
| At least one signal-conditioned Analog input  | Water Sensor |
| One remote input | HM-10 Bluetooth |

### C: Outputs
| Requirement | Fulfillment |
|-------------|-------------|
| At least one axis of motion with speed control | Mini Servo |
| Relay or Solenoid actuation | Relay |
| At least one illumination output with brightness control | 8x8 LED Matrix |
| Another illumination output, or a sound output with pitch control | Passive Buzzer |
| One remote output | Hm-10 Bluetooth |

### D: Internals
| Requirement | Fulfillment |
|-------------|-------------|
| Use a new library | MRC522 RFID Library |
| Use non-volatile storage | EEPROM |

## References
- Goodheart, M. "Lab 11."[Source Code] Section A01, Digital Control w/ Embedded Systems, University of Vermont, Burlington VT, May 7, 2024.  
- Balboa, Miguel. “Miguelbalboa/RFID: Arduino Rfid Library for MFRC522.” GitHub, 7 Dec. 2023, github.com/miguelbalboa/rfid?tab=readme-ov-file. 

