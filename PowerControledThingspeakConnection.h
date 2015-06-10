#ifndef POWERCONTROLEDTHINGSPEAKCONNECTION_H
#define POWERCONTROLEDTHINGSPEAKCONNECTION_H
#include "ThingspeakConnection.h"
#include "Timeout.h"
#include "Debugging.h"

enum PowerState {ON, OFF};

class PowerControledThingspeakConnection : public ThingspeakConnection {
  public:
    PowerControledThingspeakConnection(Stream &communicationStream,
                                       int gndCTRLPin,
                                       int vccCTRLPin,
                                       int resetPin) :
      ThingspeakConnection(communicationStream),
      gndCTRLPin(gndCTRLPin),
      vccCTRLPin(vccCTRLPin),
      resetPin(resetPin),
      state(OFF)
    {
      pinMode(gndCTRLPin, OUTPUT);
      pinMode(vccCTRLPin, OUTPUT);
      pinMode(resetPin, OUTPUT);
    }

    boolean pushToThingSpeak(int retries, float bat, int sensor1, int sensor2, unsigned long estTime, unsigned long thisPushT, unsigned long nextPushT) {
      debug("Pushing data to thingspeak power controlled");
      while (retries > 0) {
        //Give him time to wake up also this prevents sending more than one push within 15 seconds because that is the maximum of thingspeak
        boolean pushSuccess = tryPushToThingSpeak(bat, sensor1, sensor2, estTime, thisPushT, nextPushT);
        if (pushSuccess)
          return true;
        else
          retries--;
      }
      debug("Pushing data Failed");
      return false;
    }

    boolean tryPushToThingSpeak(float bat, int sensor1, int sensor2, unsigned long estTime, unsigned long thisPushT, unsigned long nextPushT) {
      debug("Trypush of power controled");
      if (state == OFF)
        if (!enableModem())
          return false;

      bool pushSuccess = ThingspeakConnection::tryPushToThingSpeak(bat, sensor1, sensor2, estTime, thisPushT, nextPushT);
      disableModem();
      return pushSuccess;
    }


    //Call this in the main loop. Then write e on serial to enable and d to disable
    void manualModemControlLoop() {
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

    bool enableModem() {
      digitalWrite(gndCTRLPin, HIGH);
      digitalWrite(vccCTRLPin, HIGH);
      digitalWrite(resetPin, HIGH);
      debug("Waiting for modem to enable");
      Timeout t(SLOW_TIMEOUT);
      while (!sendCommand("ATE0", "OK\r\n", FAST_TIMEOUT)) {
        if (t.elapsed()) {
          debug("Modem does not respond. Enabeling failed!");
          state = OFF;
          digitalWrite(resetPin, LOW);
          digitalWrite(vccCTRLPin, LOW);
          digitalWrite(gndCTRLPin, LOW);
          return false;
        }
      }
      if (!sendCommand("AT+QIURC=0", "OK\r\n", FAST_TIMEOUT)) {
        debug("Could not disable the call ready signal!");
        state = OFF;
        digitalWrite(resetPin, LOW);
        digitalWrite(vccCTRLPin, LOW);
        digitalWrite(gndCTRLPin, LOW);
        return false;
      }
//      if (!waitFor("Call Ready", SLOW_TIMEOUT * 4)) {
//        debug("Got no call ready answer!!");
//        state = OFF;
//        digitalWrite(resetPin, LOW);
//        digitalWrite(vccCTRLPin, LOW);
//        digitalWrite(gndCTRLPin, LOW);
//        return false;
//      }
      debug("Enabled Modem");
      state = ON;
      return true;
    }

    void disableModem() {
      digitalWrite(gndCTRLPin, LOW);
      digitalWrite(vccCTRLPin, LOW);
      digitalWrite(resetPin, LOW);
      delay(1000); //wait a second to make sure all power is gone
      debug("Disabeled Modem");
      state = OFF;
    }

  private:
    int gndCTRLPin, vccCTRLPin, resetPin;
    PowerState state;
};

#endif // POWERCONTROLEDTHINGSPEAKCONNECTION_H
