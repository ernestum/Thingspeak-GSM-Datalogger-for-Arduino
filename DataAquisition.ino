#include <Arduino.h>

// Sensor pins and interupts
#define SENS1INTER 1
#define SENS2INTER 4
#define SENS1PIN 2
#define SENS2PIN 7
#define BATTSENSPIN A5

const int sensorPins[] = {SENS1PIN, SENS2PIN};
int lastSensorValues[] = {0, 0};

void enableSensors() {
    pinMode(SENS2PIN, INPUT_PULLUP);
    pinMode(SENS1PIN, INPUT_PULLUP);
    pinMode(BATTSENSPIN, INPUT);
}

int batteryVoltage() {
  int volt = analogRead(BATTSENSPIN);
  volt = map(volt, 0, 1024, 0, 15000) + 500; // compensate for diode drop
  return volt;
}

int lastSensor1, lastSensor2;

int  sensorValue(byte sensor) {
    lastSensorValues[sensor] = !digitalRead(sensorPins[sensor]);
    return lastSensorValues[sensor];
}

bool sensorValueChanged(byte sensor) {
    int newValue = !digitalRead(sensorPins[sensor]);
    bool isNew = newValue != lastSensorValues[sensor];
    lastSensorValues[sensor] = newValue;
    return isNew;
}

bool sensorValueChanged() {
    return sensorValueChanged(0) || sensorValueChanged(1);
}
