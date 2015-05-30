
//Call this in the main loop. Then write e on serial to enable and d to disable
void manualModemControl() {
  if (Serial.available())
    switch (Serial.read()) {
      case 'e' :
        enableModem();
        break;
      case 'd':
        disableModem();
        break;
    }
}

void enableModem() {
  digitalWrite(GNDCTRLPIN, HIGH);
  digitalWrite(VCC2TRLPIN, HIGH);
  digitalWrite(MODEM_RESET_PIN, HIGH);
#ifdef DEBUG
  debug("Enabled Modem");
#endif
}

void disableModem() {
  digitalWrite(GNDCTRLPIN, LOW);
  digitalWrite(VCC2TRLPIN, LOW);
  digitalWrite(MODEM_RESET_PIN, LOW);
#ifdef DEBUG
  debug("Disabeled Modem");
#endif
}
