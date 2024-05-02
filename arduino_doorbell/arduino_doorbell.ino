/*                              Smart Doorbell
 * -------------------------------------------------------------------------
 * This program is my final project for Digital Control w/ Embedded Systems.
 * The goal is to combine a bunch of sensor from the ELEGOO starter kit that
 * we were given and construct something useful.
 *
 * =========================================================================
 * Setup
 * -------------------------------------------------------------------------
 * Connect each pin on each device to the respective pin as specified on
 * github: https://github.com/owncook/Arduino-Doorbell/tree/main
 *
 * =========================================================================
*/

#include <SPI.h> // RFID
#include <MFRC522.h> // RFID
#include <EEPROM.h> // Memory
#include <SoftwareSerial.h> // Bluetooth
#include <LedControl.h> // 8x8 Matrix
#include <Servo.h> // Servo

/*
 * ======================================================================
 * Defining pins
 * ======================================================================
*/

// RFID
/*
 * 3.3V -> Vin
 * RST -> 9
 * GND -> GND
 * IRQ -> NOTHING
 * MISO -> 12
 * MOSI -> 11
 * SCK -> 13
 * SDA -> 10
 */
#define RFID_SS 10 // SPI Communication (SDA)
#define RFID_RST 9 // Reset pin 

// Relay
#define RELAY_PIN 2

// Bluetooth
#define RX_TO_TX 3 // From Arduino to Bluetooth
#define TX_TO_RX 4 // From Arduino to Bluetooth

// Button
#define BUTTON_PIN 8

// LED Matrix
#define LED_LATCH 6
#define LED_DATA 7
#define LED_CLOCK SCL

#define LED_NUM_MATRICES 1
#define LED_BRIGHTNESS 5

// Water sensor
#define WATER_SENSOR_INPUT A0

// Potentiometer
#define POT_INPUT A1

// Servo
#define SERVO_PIN SDA
#define LOCKED_POS 0
#define UNLOCKED_POS 90

// Ultrasonic Sensor
#define TRIG_PIN 0
#define ECHO_PIN 1 // Might have to change
#define SPEED_OF_SOUND .0343 // cm/us

// EEPROM
#define LOCKED_ADDRESS 0

// Buzzer 
#define BUZZER_PIN 5

/*
 * ======================================================================
 * Variables/Objects/Functions
 * ======================================================================
*/

// RFID
byte readCard[4];
String acceptedTag = "FAF37991"; // Tag ID from one of the RFID library examples
// Helper variables for getID function
String tagID = "";
MFRC522 rfid(RFID_SS, RFID_RST);
// 
boolean getID(); 

// Bluetooth
SoftwareSerial phoneSerial(RX_TO_TX, TX_TO_RX);

// LED Matrix
byte lockIcon[8] = {B00000000,
                    B00111100,
                    B01000010,
                    B10000001,
                    B10000001,
                    B11111111,
                    B11111111,
                    B11111111};

byte unlockIcon[8] = {B00111100,
                      B01000010,
                      B10000001,
                      B10000001,
                      B00000001,
                      B11111111,
                      B11111111,
                      B11111111};

LedControl ledMatrix = LedControl(LED_DATA, LED_CLOCK, LED_LATCH, LED_NUM_MATRICES);

// Servo
Servo lockServo;
int servoPosition;
int servoSpeed = 90;

// Moisture Sensor
int moisture;
bool raining;

// Ultrasonic Sensor
float time_signal_out;
float time_seconds;
float distance;

// Potentiometer
int potReading;

// Buzzer
int volume = 500;

// General logic variables
bool locked = true;
bool someoneAtDoor = false;
bool doorBellRung = false;
bool message;

void setup() {

  // RFID
  SPI.begin();
  rfid.PCD_Init();
  delay(4);

  // Relay
  pinMode(RELAY_PIN, OUTPUT);

  // Bluetooth
  phoneSerial.begin(9600);

  // LED Matrix
  ledMatrix.shutdown(0,false); // Turn on
  ledMatrix.setIntensity(0,LED_BRIGHTNESS); // Set brightness
  ledMatrix.clearDisplay(0); // Clear display

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // On power up, door kept at previous lock state, else locks door
  locked = (bool)EEPROM.read(LOCKED_ADDRESS);

  // Make sure servo is in right state upon power
  lockServo.attach(SERVO_PIN);
  if (locked) {
    lockServo.write(0);
  } else {
    lockServo.write(90);
  }
  delay(100);
  lockServo.detach();

  // Menu message
  phoneSerial.write("\n===========================\n");
  phoneSerial.write("Type 'L' for lock, 'U' for unlock, or 'S' for settings\n");
  phoneSerial.write("===========================\n");

}

void loop() {
  message = false; // Deals with 

  // Read input from phone
  while (phoneSerial.available() > 0) {
    char input = tolower(phoneSerial.read());
    switch (input) {
      case ('l'): {
        locked = true;
        phoneSerial.write("Door Locked\n");
        EEPROM.write(LOCKED_ADDRESS, (int)locked);
        message = true;
        break;
      }
      case ('u'): {
        locked = false;
        phoneSerial.write("Door Unlocked\n");
        EEPROM.write(LOCKED_ADDRESS, (int)locked);
        message = true;
        break;
      }
      case ('s'): {
        phoneSerial.write("Enter 'S' to change servo speed or 'T' to change doorbell tone\n");
        while (phoneSerial.available() == 0);
        while (phoneSerial.available() > 0) {
          char settingsChoice = tolower(phoneSerial.read());
          switch (settingsChoice) {
            case ('s'): {
              phoneSerial.write("Enter 'f', 'm', or 's' for fast, medium, or slow servo speed\n");
              while (phoneSerial.available() == 0);
              while (phoneSerial.available() > 0) {
                char speedInput = tolower(phoneSerial.read());
                switch (speedInput) {
                  case ('f'): {
                    servoSpeed = 90;
                    break;
                  }
                  case ('m'): {
                    servoSpeed = 15;
                    break;
                  }
                  case ('s'): {
                    servoSpeed = 6;
                    break;
                  }
                }
              }
              break;
            }

            case ('t'): {
              phoneSerial.write("Enter a value of volume for doorbell between 50-2000\n");
              message = true;
              while(phoneSerial.available() == 0);
              while(phoneSerial.available() > 0) {
                volume = phoneSerial.parseInt();
                if (volume > 2000) {
                  volume = 2000;
                } else if (volume < 50) {
                  volume = 50;
                }
              }
              break;
            }
          }
        }
        // Confirmation message
        phoneSerial.write("Task successful, exiting settings...\n");
        message = true;
        break;
      }
    }
  }

  // Unlocks door with RFID Tag
  if (getID()) {
    if (locked && tagID == acceptedTag) {
      phoneSerial.write("Door Unlocked\n");
      message = true;
      locked = false;
      EEPROM.write(LOCKED_ADDRESS, (int)locked);
    }
  }

  // Showing Lock/Unlock Icon and turning servo
  lockServo.attach(SERVO_PIN); // Servo was jittering if it wasn't attached and detached
  if (locked) {
    // Setting each row to each byte in respective icon
    for(int i = 0; i < 8; i++) {
      ledMatrix.setRow(0, i, lockIcon[i]);
    }
    if (servoPosition == UNLOCKED_POS) {
      turnServo();
    }

  } else {
    for(int i = 0; i < 8; i++) {
      ledMatrix.setRow(0, i, unlockIcon[i]);
    }
    if (servoPosition == LOCKED_POS) {
      turnServo();
    }
  }
  delay(100);
  lockServo.detach();

  // Buzzes if button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    someoneAtDoor = false;
    tone(BUZZER_PIN, volume);

    // Makes sure only one message is sent per press
    if (!doorBellRung) {
      phoneSerial.write("Doorbell Rung\n");
      message = true;
      doorBellRung = true;
    }
  } else {
    noTone(BUZZER_PIN);
    doorBellRung = false;
  }

  if (!someoneAtDoor) {
    // Turns off LED when no one is at the door
    digitalWrite(RELAY_PIN, LOW);
    // Ultrasonic sensor send sound for 10 us
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Read sound and divide by two because only want 1 way
    time_signal_out = pulseIn(ECHO_PIN, HIGH) / 2;

    // Distance in cm
    distance = time_signal_out * SPEED_OF_SOUND;

    // If someone is close enough to door
    if (distance < 5.00) {
      phoneSerial.write("Someone detected at the door\n");
      message = true;
      someoneAtDoor = true;
    }
  } else {
    // Turns on LED when someone at door
    digitalWrite(RELAY_PIN, HIGH);
  }

  // Raining notification
  moisture = analogRead(WATER_SENSOR_INPUT);
  if (moisture < 300 && !raining) {
    phoneSerial.write("It's raining\n");
    message = true;
    raining = true;
  } else if (moisture >= 300) {
    raining = false;
  }

  // Setting brightness of LED Matrix
  potReading = (int)floor(analogRead(POT_INPUT) / 100); // Getting values 0-10
  ledMatrix.setIntensity(0, potReading);

  // Adds a divider between messages if there was a message
  if (message) {
    phoneSerial.write("===========================\n");
  }

}

// Gets ID of RFID tag that is scanned
// Function from MRC522 Arduino library. Source: https://github.com/miguelbalboa/rfid/blob/master/examples/DumpInfo/DumpInfo.ino
boolean getID() {

  // Getting ready for Reading PICCs
  // If a new PICC placed to RFID reader continue
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return false;
  }

  // Since a PICC placed get Serial and continue
  if ( ! rfid.PICC_ReadCardSerial()) {
    return false;
  }

  tagID = "";
  // The MIFARE PICCs that we use have 4 byte UID
  for ( uint8_t i = 0; i < 4; i++) {

    // Adds the 4 bytes in a single String variable
    tagID.concat(String(rfid.uid.uidByte[i], HEX));
  }

  tagID.toUpperCase();
  rfid.PICC_HaltA(); // Stop reading
  return true;

}

// Turns servo 90º to either position 0 or 90 at varying speeds
void turnServo() {

  int endAngle = abs(90 - servoPosition);
  servoPosition = lockServo.read();

  while (servoPosition != endAngle) {
    if (endAngle == LOCKED_POS) {
      servoPosition -= servoSpeed;
    } else {
      servoPosition += servoSpeed;
    }
    lockServo.write(servoPosition);
    delay(250);
  }

}

