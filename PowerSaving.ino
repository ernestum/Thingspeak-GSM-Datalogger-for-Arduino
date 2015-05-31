#include <avr/sleep.h>
#include <avr/power.h>

volatile boolean sensor1Changed = false, sensor2Changed = false;

void sensor1Interrupt(void) {
  detachInterrupt(SENS1INTER);
  detachInterrupt(SENS2INTER);
  sensor1Changed = true;
}

void sensor2Interrupt(void) {
  detachInterrupt(SENS1INTER);
  detachInterrupt(SENS2INTER);
  sensor2Changed = true;
}

void enableWatchdog() {
  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);
}

void gotoSleep() {
  attachInterrupt(SENS1INTER, sensor1Interrupt, LOW); //THe logic is inverted
  attachInterrupt(SENS1INTER, sensor1Interrupt, LOW);
  delay(100);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  sleep_enable();

  sleep_mode();

  /* The program will continue from here. */

  /* First thing to do is disable sleep. */
  sleep_disable();
  power_all_enable();
}



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
