#include <Arduino.h>

/// This file contains some helper functions to access the attached sensors and
/// the battery voltage

// Sensor pins and interupts
#define SENS1INTER 1
#define SENS2INTER 4
#define SENS1PIN 2
#define SENS2PIN 7
#define BATTSENSPIN A5

const int sensorPins[] = {SENS1PIN, SENS2PIN};
int lastSensorValues[] = {0, 0};

/// Sets up the pins for the sensors
void enableSensors() {
  pinMode(SENS2PIN, INPUT_PULLUP);
  pinMode(SENS1PIN, INPUT_PULLUP);
  pinMode(BATTSENSPIN, INPUT);
}

/// Returns the current battery volage in mV
int batteryVoltage() {
  int volt = analogRead(BATTSENSPIN);
  volt = map(volt, 0, 1024, 0, 15000) + 500;  // compensate for diode drop
  return volt;
}

/// Unused variables to be deleted in the next release
int lastSensor1, lastSensor2;

/// Reads the current value of the given sensor (0 or 1)
/// Also stores that value as the last known sensor value
int sensorValue(byte sensor) {
  lastSensorValues[sensor] = !digitalRead(sensorPins[sensor]);
  return lastSensorValues[sensor];
}

/// Checks if the sensor value of a given sensor (0 or 1) changed
/// since we checked it last time.
bool sensorValueChanged(byte sensor) {
  int newValue = !digitalRead(sensorPins[sensor]);
  bool isNew = newValue != lastSensorValues[sensor];
  lastSensorValues[sensor] = newValue;
  return isNew;
}

/// Checks if any of the two sensors changed since we checked last time
bool sensorValueChanged() {
  return sensorValueChanged(0) || sensorValueChanged(1);
}
