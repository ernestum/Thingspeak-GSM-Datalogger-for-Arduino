#ifndef POWERCONTROLEDTHINGSPEAKCONNECTION_H
#define POWERCONTROLEDTHINGSPEAKCONNECTION_H
#include "ThingspeakConnection.h"
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

    }

    boolean tryPushToThingSpeak(float bat, int sensor1, int sensor2) {
        if(state == OFF)
            enableModem();
        ThingspeakConnection::tryPushToThingSpeak(bat, sensor1, sensor2);
        disableModem();
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

    void enableModem() {
//        digitalWrite(gndCTRLPin, HIGH);
//        digitalWrite(vccCTRLPin, HIGH);
        digitalWrite(resetPin, HIGH);
        //TODO: Wait for connection ready here
        debug("Enabled Modem");
        state = ON;
    }

    void disableModem() {
//      digitalWrite(gndCTRLPin, LOW);
//      digitalWrite(vccCTRLPin, LOW);
      digitalWrite(resetPin, LOW);
      debug("Disabeled Modem");
      state = OFF;
    }

private:
    int gndCTRLPin, vccCTRLPin, resetPin;
    PowerState state;
};

#endif // POWERCONTROLEDTHINGSPEAKCONNECTION_H
