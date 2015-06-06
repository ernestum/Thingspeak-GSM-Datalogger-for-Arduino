#include <avr/sleep.h>
#include <avr/power.h>

/* Links:
http://shelvin.de/arduino-in-den-sleep_mode_pwr_down-schlaf-modus-setzen/
http://www.mikrocontroller.net/articles/Sleep_Mode
http://donalmorrissey.blogspot.nl/2010/04/sleeping-arduino-part-5-wake-up-via.html
*/

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

ISR(WDT_vect)
{
  Serial.println("WATCHDOC!");
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
  //attachInterrupt(SENS1INTER, sensor1Interrupt, LOW); //THe logic is inverted
  //attachInterrupt(SENS2INTER, sensor2Interrupt, LOW);
  //delay(100);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  sleep_enable();
  Serial.println("Going to sleep!");
    Serial.flush();

  sleep_mode();

  /* The program will continue from here. */

  /* First thing to do is disable sleep. */
  sleep_disable();
  power_all_enable();
//  Serial.begin(19200);
//  while(!Serial) {}
  Serial.println("Woke up from sleep!");
}



