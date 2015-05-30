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
