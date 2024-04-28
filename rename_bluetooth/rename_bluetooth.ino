#include <SoftwareSerial.h>
SoftwareSerial mySerial(3,2);  // TX is on pin8 and RX on pin 9

void setup() {
  mySerial.begin(9600);   
  Serial.begin(9600);   
  delay(100);

  mySerial.write("AT\r\n");  // put HM10 in AT command mode
  delay(100);
  mySerial.write("AT+NAME YOURNAMEHERE\r\n");  // Name our HM10 something so as to not interfere with others
  delay(100);
  mySerial.write("AT+NAME\r\n");  // Verify new name
  delay(100);
  mySerial.write("AT+RESET\r\n");  // reset HM10 so new name will take effect
}

/* 
  This loop allows you to input any AT commands. But after a BLE connection you can't send AT commands.
  You can modify this loop to look for characters from your BLE phone app and to send data to your 
  phone app
*/
void loop() {

if (Serial.available()>0)
mySerial.write(Serial.read());
if (mySerial.available()>0)
Serial.write(mySerial.read());

}
