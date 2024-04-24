
#include <SPI.h> // RFID
#include <MFRC522.h> // RFID
#include <EEPROM.h> // Memory
#include <SoftwareSerial.h> // Bluetooth
#include <LedControl.h> // 8x8 Matrix
#include <Servo.h> // Servo
#include <TimeLib.h> // Current time

// Relay
#define RELAY_PIN 1

// Bluetooth
#define TX_TO_RX 2 // From Arduino to Bluetooth
#define RX_TO_TX 3 // From Arduino to Bluetooth

// Ultrasonic Sensor
#define TRIG_PIN 4
#define ECHO_PIN 5
#define SPEED_OF_SOUND .0343 // cm/us

// LED Matrix
#define LED_LATCH 6
#define LED_DATA 7
#define LED_CLK SCL

// Button
#define BUTTON_PIN 8

#define NUM_LED_MATRICES 1
#define LED_BRIGHTNESS 5

// RFID
#define RFID_RST 9
#define RFID_SDA 10

// Water Sensor 
#define WATER_SENSOR_IN A0

// EEPROM
#define EEPROM_ADDRESS 0

// Servo
#define SERVO_PIN SDA

/*
 * Variables/Objects
 */

// RFID
byte readCard[4];
String acceptedTag = "FAF37991";
String tagID = "";
MFRC522 rfid(RFID_SDA, RFID_RST);
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

LedControl ledMatrix = LedControl(LED_DATA, LED_CLK, LED_LATCH, NUM_LED_MATRICES);

// Servo
Servo lockServo;

// Ultrasonic Sensor
float time_signal_out;
float time_seconds;
float distance;

// Clock
// Hour, minute, second, month, day, year
int currTime[6] = {7, 50, 30, 4, 7, 2024};
void printTimeStamp(); // Prints current time to phone (used after every event)

// Moisture Sensor
int moisture;
bool raining = false;

// General logic variables
bool locked;
String previousLockState;
bool someoneAtDoor = false;
bool doorBellRung = false;

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

  // Initialize clock with current time
  setTime(currTime[0], currTime[1], currTime[2], currTime[3], currTime[4], currTime[5]);

  // Read last locked value from EEPROM
  // byte boolChecker = EEPROM.read(ADDRESS);
  // bool lastLockVal = (bool)boolChecker;

  // On power up, door kept at previous lock state, else locks door
  // if (lastLockVal == false || lastLockVal == true) {
  //   locked = lastLockVal;
  // } else {
  //   locked = true;
  // }

  phoneSerial.write("\nType L for lock and U for unlock\n");

}

void loop() {

  // Update time
  currTime[0] = hour();
  currTime[1] = minute();
  currTime[2] = second();
  currTime[3] = month();
  currTime[4] = day();
  currTime[5] = year();

  // Read input from phone
  while (phoneSerial.available() > 0) {
    char input = tolower(phoneSerial.read());
    switch (input) {
      case ('l'): {
        locked = true;
        // EEPROM.write(ADDRESS, locked);
        phoneSerial.write("Door Locked");
        printTimeStamp();
        break;
      }
      case ('u'): {
        locked = false;
        // EEPROM.write(ADDRESS, locked);
        phoneSerial.write("Door Unlocked");
        printTimeStamp();
        break;
      }
    }
  }

  // Unlocks door with RFID Tag
  if (getID()) {
    if (locked && tagID == acceptedTag) {
      phoneSerial.write("Door Unlocked");
      printTimeStamp();
      locked = false;
      // EEPROM.write(ADDRESS, locked);
    }
  }

  // Showing Lock/Unlock Icon and turning servo
  lockServo.attach(SERVO_PIN);
  if (locked) {
    for(int i = 0; i < 8; i++) {
      ledMatrix.setRow(0, i, lockIcon[i]);
    }
    lockServo.write(0);
  
  } else {
    for(int i = 0; i < 8; i++) {
      ledMatrix.setRow(0, i, unlockIcon[i]);
    }
    lockServo.write(90);
  }
  lockServo.detach();

  // Buzzes if button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    digitalWrite(RELAY_PIN, HIGH);
    someoneAtDoor = false;
    if (!doorBellRung) {
      phoneSerial.write("Doorbell Rung");
      printTimeStamp();
      doorBellRung = true;
    }
  } else {
    digitalWrite(RELAY_PIN, LOW);
    doorBellRung = false;
  }

  // Raining notification
  moisture = analogRead(WATER_SENSOR_IN);
  if (moisture < 300 && !raining) {
    phoneSerial.write("It's raining");
    printTimeStamp();
    raining = true;
  } else if (moisture >= 300) {
    raining = false;
  }

  if (!someoneAtDoor) {
    // Ultrasonic sensor send sound for 10 us
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Read sound and divide by two because only want 1 way
    time_signal_out = pulseIn(ECHO_PIN, HIGH) / 2;

    // Distance in cm
    distance = time_signal_out * SPEED_OF_SOUND;

    if (distance < 5.00) {
      phoneSerial.write("Someone detected at the door");
      printTimeStamp();
      someoneAtDoor = true;
    }
  }
  
}


// Gets ID of RFID tag that is scanned
boolean getID() {

  // Getting ready for Reading PICCs
  //If a new PICC placed to RFID reader continue
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return false;
  }

  //Since a PICC placed get Serial and continue
  if ( ! rfid.PICC_ReadCardSerial()) {
    return false;
  }

  tagID = "";
  // The MIFARE PICCs that we use have 4 byte UID
  for ( uint8_t i = 0; i < 4; i++) {
    //readCard[i] = mfrc522.uid.uidByte[i];
    // Adds the 4 bytes in a single String variable
    tagID.concat(String(rfid.uid.uidByte[i], HEX));
  }

  tagID.toUpperCase();
  rfid.PICC_HaltA(); // Stop reading
  return true;

}

void printTimeStamp() {
  // Stores the current time as a string to pass to the phone
  char time_str[6][4]; 
  for(int i = 0; i < 6; i++) {
    sprintf(time_str[i], "%d", currTime[i]);
  }
  phoneSerial.write(" | ");
  phoneSerial.write(time_str[0]);
  phoneSerial.write(':');
  phoneSerial.write(time_str[1]);
  phoneSerial.write(':');
  phoneSerial.write(time_str[2]);
  phoneSerial.write(' ');
  phoneSerial.write(time_str[3]);
  phoneSerial.write('/');
  phoneSerial.write(time_str[4]);
  phoneSerial.write('/');
  phoneSerial.write(time_str[5]);
  phoneSerial.write("\n");

}




