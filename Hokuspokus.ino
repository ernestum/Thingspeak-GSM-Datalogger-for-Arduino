
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

int batteryVoltage(){
  int volt = analogRead(A5);
  volt = map(volt, 0, 1024, 0, 15000) + 500; // compensate for diode drop
  return volt;
}

int  sensorValue(int sensor){
  if (sensor){
    return !digitalRead(SENS2PIN);
  }
  return !digitalRead(SENS1PIN);
}

void setup()
{
  // configure some pins
  pinMode(GNDCTRLPIN, OUTPUT);
  pinMode(VCC2TRLPIN, OUTPUT);
  pinMode(SENS1PIN, INPUT_PULLUP);
  pinMode(SENS2PIN, INPUT_PULLUP);
}

void loop() {
}

