#define DEBUG

#include "PowerControledThingspeakConnection.h"
// Controll pins
#define GNDCTRLPIN 12
#define VCC2TRLPIN 9
#define MODEM_RESET_PIN 4

// Sensor pins and interupts
#define SENS1INTER 1
#define SENS2INTER 4
#define SENS1PIN 2
#define SENS2PIN 7
#define BATTSENSPIN A5



//http://api.thingspeak.com/update?api_key=YDHVCBHPKX9TOPXF&field1=BATTERY&field2=SENSOR1&field3=SENSOR2

PowerControledThingspeakConnection modem(Serial1, GNDCTRLPIN, VCC2TRLPIN, MODEM_RESET_PIN);

void setup()
{
  Serial.begin(9600);
  while (!Serial) {} //Needs to be commented out when running in "headless" mode (without a serial monitor open)

  Serial1.begin(9600);

//  modem.pushToThingSpeak(0, 0, 0);

  Serial.println("Finished Listening");
  Serial.flush();

  // configure some pins
  pinMode(GNDCTRLPIN, OUTPUT);
  pinMode(VCC2TRLPIN, OUTPUT);
  pinMode(MODEM_RESET_PIN, OUTPUT);

  digitalWrite(GNDCTRLPIN, HIGH);
  digitalWrite(VCC2TRLPIN, HIGH);

  pinMode(SENS1PIN, INPUT_PULLUP);
  pinMode(SENS2PIN, INPUT_PULLUP);

  modem.enableModem();
  

}

void readAllFromSerial1(int timeout) {
  unsigned long start = millis();
  while (millis() - start < timeout)
    while (Serial1.available() > 0)
      Serial.print((char)Serial1.read());
}



void loop() {
//    modem.manualModemControlLoop();
    modemSerialBridge();
}

void modemSerialBridge() {
    char incoming_char;
    if (Serial1.available() > 0)
    {
      incoming_char = Serial1.read();
      Serial.print(incoming_char);
//      printCharDetail(incoming_char);
      //    if (incoming_char == 10)
      //      Serial.println();
      //    else
      //      Serial.print(incoming_char);
    }

    if (Serial.available() > 0)
    {
      incoming_char = Serial.read();
      Serial1.print(incoming_char);
    }
}



