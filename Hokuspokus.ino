#include <SoftwareSerial.h>

SoftwareSerial cell(8, 3); // RX, TX

// Controll pins
#define GNDCTRLPIN 12
#define VCC2TRLPIN 9

// Sensor pins and interupts
#define SENS1INTER 1
#define SENS2INTER 4
#define SENS1PIN 2
#define SENS2PIN 7
#define BATTSENSPIN A5

//http://api.thingspeak.com/update?api_key=YDHVCBHPKX9TOPXF&field1=BATTERY&field2=SENSOR1&field3=SENSOR2

int batteryVoltage() {
  int volt = analogRead(A5);
  volt = map(volt, 0, 1024, 0, 15000) + 500; // compensate for diode drop
  return volt;
}

int  sensorValue(int sensor) {
  if (sensor) {
    return !digitalRead(SENS2PIN);
  }
  return !digitalRead(SENS1PIN);
}


void printThingspeakString(float bat, int sensor1, int sensor2) {
  Serial.print("http://api.thingspeak.com/update?api_key=YDHVCBHPKX9TOPXF&field1=");
  Serial.print(bat);
  Serial.print("&field2=");
  Serial.print(sensor1);
  Serial.print("&field3=");
  Serial.println(sensor2);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial) {}
  cell.begin(9600);
  Serial.print("Starting modem communication...");
  delay(5000);
  Serial.print("OK\nIntroduce your AT commands:\n");
  cell.println("ATE1");
  readAllFromCell(1000);
  cell.println("AT+QICLOSE");
  readAllFromCell(1000);
  cell.println("AT+QIREGAPP=\"web.be\",\"web\",\"web\"");
  readAllFromCell(1000);
  cell.println("AT+QINDI=0");
  readAllFromCell(1000);
  cell.println("AT+QIOPEN=\"TCP\",\"184.106.153.149\",80");
  readAllFromCell(5000);
  cell.println("AT+QISEND");
  readAllFromCell(100);
  cell.println("GET /update?api_key=YDHVCBHPKX9TOPXF&field1=42&field2=5&field3=5 HTTP/1.0\n\n\x1A");
  //cell.println("GET / HTTP/1.0\n\n\x1A");
  readAllFromCell(50000);
  
  Serial.println("Finished Listening");
 
// configure some pins
  //pinMode(GNDCTRLPIN, OUTPUT);
  //pinMode(VCC2TRLPIN, OUTPUT);
  //pinMode(SENS1PIN, INPUT_PULLUP);
  //pinMode(SENS2PIN, INPUT_PULLUP);

}

void readAllFromCell(int timeout) {
  unsigned long start = millis();
  while (millis() - start < timeout)
    while (cell.available() > 0)
      Serial.print((char)cell.read());
}

void loop() {
  //printThingspeakString(batteryVoltage(), sensorValue(0), sensorValue(1));
  //delay(1000);
  char incoming_char;
   if(cell.available() > 0)
  {
    incoming_char = cell.read();
    if (incoming_char == 10)
      Serial.println();
    else
      Serial.print(incoming_char);
  }

  if (Serial.available() > 0)
  {
    incoming_char = Serial.read();
    cell.print(incoming_char);
  }
}

