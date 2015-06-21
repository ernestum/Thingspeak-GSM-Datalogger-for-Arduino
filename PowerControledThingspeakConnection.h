#ifndef POWERCONTROLEDTHINGSPEAKCONNECTION_H
#define POWERCONTROLEDTHINGSPEAKCONNECTION_H
#include "ThingspeakConnection.h"
#include "Timeout.h"
#include "Debugging.h"

enum PowerState { ON, OFF };

class PowerControledThingspeakConnection : public ThingspeakConnection {
 public:
  PowerControledThingspeakConnection(Stream &communicationStream,
                                     int gndCTRLPin, int vccCTRLPin,
                                     int resetPin)
      : ThingspeakConnection(communicationStream),
        gndCTRLPin(gndCTRLPin),
        vccCTRLPin(vccCTRLPin),
        resetPin(resetPin),
        state(OFF) {
    pinMode(gndCTRLPin, OUTPUT);
    pinMode(vccCTRLPin, OUTPUT);
    pinMode(resetPin, OUTPUT);
  }

  boolean pushToThingSpeak(int retries, float bat, int sensor1, int sensor2) {
    D_MSG(1, "Push data to thingspeak");
    while (retries > 0) {
      boolean pushSuccess = tryPushToThingSpeak(bat, sensor1, sensor2);
      if (pushSuccess) {
        D_MSG(1, "Data pushed successfully!");
        return true;
      } else {
        retries--;
        D_MSG(1, "Pushing data failed but we retry.");
      }
    }
    D_MSG(1, "Pushing data failed completely; we give up for now");
    return false;
  }

  boolean tryPushToThingSpeak(float bat, int sensor1, int sensor2) {
    D_MSG(2, "Enable power before we try to push");
    if (state == OFF)
      if (!enableModem()) return false;

    bool pushSuccess =
        ThingspeakConnection::tryPushToThingSpeak(bat, sensor1, sensor2);
    disableModem();
    return pushSuccess;
  }

  bool enableModem() {
    digitalWrite(gndCTRLPin, HIGH);
    digitalWrite(vccCTRLPin, HIGH);
    digitalWrite(resetPin, HIGH);
    D_MSG(3, "Waiting for modem to enable");
    Timeout t(SLOW_TIMEOUT);
    while (!sendCommand("ATE0", "OK\r\n", FAST_TIMEOUT)) {
      if (t.elapsed()) {
        D_MSG(3, "Modem does not respond. Enabeling failed!");
        state = OFF;
        digitalWrite(resetPin, LOW);
        digitalWrite(vccCTRLPin, LOW);
        digitalWrite(gndCTRLPin, LOW);
        return false;
      }
    }
    if (!sendCommand("AT+QIURC=0", "OK\r\n", FAST_TIMEOUT)) {
      D_MSG(3, "Could not disable the call ready signal!");
      state = OFF;
      digitalWrite(resetPin, LOW);
      digitalWrite(vccCTRLPin, LOW);
      digitalWrite(gndCTRLPin, LOW);
      return false;
    }
    D_MSG(3, "Enabled Modem");
    state = ON;
    return true;
  }

  void disableModem() {
    digitalWrite(gndCTRLPin, LOW);
    digitalWrite(vccCTRLPin, LOW);
    digitalWrite(resetPin, LOW);
    delay(1000);  // wait a second to make sure all power is gone
    D_MSG(2, "Disabeled Modem");
    state = OFF;
  }

 private:
  int gndCTRLPin, vccCTRLPin, resetPin;
  PowerState state;
};

#endif  // POWERCONTROLEDTHINGSPEAKCONNECTION_H
