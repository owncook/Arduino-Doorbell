/*                         Smart Doorbell
 *
 * This program is my final project for Digital Control w/ Embedded Systems.
 * The goal is to combine a bunch of sensor from the ELEGOO starter kit that
 * we were given and construct something useful.
 * 
 *
 * NOTE HAVE TO UPLOAD CODE THEN TURN ON THE POWER SUPPLY
 * ===========================================================================
 * Sensors and Components Used:
 * ----------------------------
 * 1. RFID Sensor
 * 2. Ultrasonic Sensor
 * 3. Relay Module
 * 4. Water Sensor
 * 5. 9G Mini Servo
 * 6. HM-10 Bluetooth Module
 * 7. 8x8 LED Matrix
 * 8. Active Buzzer
 * 9. Arduino Uno R3
 * ----------------------------
 * ===========================================================================
 * Component Pinouts
 * ------------------------------------
 *                                  Elegoo
 * Component                        Arduino R3
 * ------------------------------------
 * RFID Sensor
 * Ultrasonic Sensor
 * Relay Module
 * Water Sensor
 * 9G Mini Servo
 * HM-10 Bluetooth Module
 * 8x8 LED Matrix
 * Active Buzzer
 * Arduino Uno R3
*/

#include <SPI.h> // RFID
#include <MFRC522.h> // RFID
#include <EEPROM.h> // Memory
#include <SoftwareSerial.h> // Bluetooth
#include <LedControl.h> // 8x8 Matrix
#include <Servo.h> // Servo

/*
 * Defining pins
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
#define RELAY_PIN 0

// Bluetooth
#define RX_TO_TX 3 // From Arduino to Bluetooth
#define TX_TO_RX 4 // From Arduino to Bluetooth

// Button
#define BUTTON_PIN 2

// LED Matrix
#define LED_LATCH 6
#define LED_DATA 7
#define LED_CLOCK SCL

#define LED_NUM_MATRICES 1
#define LED_BRIGHTNESS 5

// Water sensor
#define WATER_SENSOR_INPUT A0

// Servo
#define SERVO_PIN SDA

// Ultrasonic Sensor
#define TRIG_PIN 5
#define ECHO_PIN 8 // Might have to change
#define SPEED_OF_SOUND .0343 // cm/us

// EEPROM
#define ADDRESS 0

/*
 * Variables/Objects
 */

// RFID
byte readCard[4];
String acceptedTag = "FAF37991"; // Tag ID from one of the RFID library examples
String tagID = "";
MFRC522 rfid(RFID_SS, RFID_RST);
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

// Moisture Sensor
int moisture;
bool raining;

// Ultrasonic Sensor
float time_signal_out;
float time_seconds;
float distance;


// General logic variables
bool locked;
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

  // On power up, door kept at previous lock state, else locks door
  locked = (bool)EEPROM.read(0);

  phoneSerial.write("\nType L for lock and U for unlock\n");

}

void loop() {

  // Read input from phone
  while (phoneSerial.available() > 0) {
    char input = tolower(phoneSerial.read());
    switch (input) {
      case ('l'): {
        locked = true;
        phoneSerial.write("Door Locked");
        EEPROM.write(ADDRESS, (int)locked);
        break;
      }
      case ('u'): {
        locked = false;
        phoneSerial.write("Door Unlocked");
        EEPROM.write(ADDRESS, (int)locked);
        break;
      }
    }
  }

  // Unlocks door with RFID Tag
  if (getID()) {
    if (locked && tagID == acceptedTag) {
      phoneSerial.write("Door Unlocked");
      locked = false;
      EEPROM.write(ADDRESS, (int)locked);
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
  delay(100);
  lockServo.detach();

  // Buzzes if button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    digitalWrite(RELAY_PIN, HIGH);
    someoneAtDoor = false;
    if (!doorBellRung) {
      phoneSerial.write("Doorbell Rung");

      doorBellRung = true;
    }
  } else {
    digitalWrite(RELAY_PIN, LOW);
    doorBellRung = false;
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
      // printTimeStamp();
      someoneAtDoor = true;
    }
  }


  // Raining notification
  moisture = analogRead(WATER_SENSOR_INPUT);
  if (moisture < 300 && !raining) {
    phoneSerial.write("It's raining");
    //printTimeStamp();
    raining = true;
  } else if (moisture >= 300) {
    raining = false;
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

    // Adds the 4 bytes in a single String variable
    tagID.concat(String(rfid.uid.uidByte[i], HEX));
  }

  tagID.toUpperCase();
  rfid.PICC_HaltA(); // Stop reading
  return true;

}
