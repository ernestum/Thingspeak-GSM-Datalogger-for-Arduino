// External includes
#include <SoftwareSerial.h>
#include <LowPower.h>  //get it from here: https://github.com/rocketscream/Low-Power

/// If you want lots of debugging output on the Serial interface, define
/// DEBUG_BUILD
/// But be aware, that if you enable this and do NOT connect the serial
/// interface
/// (by connecting an USB cable and opening the serial console) the program can
/// get stuck or slow.
//#define DEBUG_BUILD

// Internal includes
#include "Debugging.h"
#include "PowerControledThingspeakConnection.h"
#include "Queue.h"

// Controll pins for the modem
#define GNDCTRLPIN 12
#define VCC2TRLPIN 9
#define MODEM_RESET_PIN 4
#define MODEM_SOFT_SERIAL_RX 8
#define MODEM_SOFT_Serial_TX 3

// Parameters for regular transmission and failure recovery strategy
#define NUM_TRANSMISSION_RETRIES 5
#define DELAY_AFTER_TRANSMISSION_FAILURE 60 * 60 * 5  // 5 hours in seconds
#define REGULAR_UPDATE_INTERVAL 60 * 60 * 22  // 22 hours in seconds in seconds
#define QSIZE 5 // Size of the outgoing queue for thingspeak updates

// This serial interface is used to communicate with the modem
SoftwareSerial modemSerial(MODEM_SOFT_SERIAL_RX,
                           MODEM_SOFT_Serial_TX);  // RX, TX

// Here we set up an interface to a power controled GSM modem
PowerControledThingspeakConnection modem(modemSerial, GNDCTRLPIN, VCC2TRLPIN,
                                         MODEM_RESET_PIN);

// The estimated number of seconds elapsed since startup
unsigned long elapsedSeconds = 0;

// The time (in seconds) of the next regular update to thingspeak
unsigned long nextRegularUpdate = 0;

// The next irregular update because some previous transmission failed
unsigned long nextRetryUpdate = 0;

/// In regular mode we send out updates to thingpseak in regular
/// intervals or if any of the sensors change. If a transmission
/// fails we transition to WAIT_AFTER_FAIL mode.

/// In WAIT_AFTER_FAIL mode we do not transmit any data until
/// DELAY_AFTER_TRANSMISSION_FAILURE seconds have passed. Any
/// changes of the sensor values will be cached in a queue until then.
/// As soon as a new transmission attempt is successfull we transition
/// back to REGULAR mode.

// The current mode we are operating in. This can be either REGULAR or
// WAIT_AFTER_FAIL
enum OperationMode {REGULAR, WAIT_AFTER_FAIL};
OperationMode mode = REGULAR;

// Struct to store a sensor reading in a queue
typedef struct {
  int sensor1;
  int sensor2;
} SensorReading;

// A queue to store unsent sensor readings.
Queue<QSIZE, SensorReading> sensorReadingQueue;

void setup() {
#ifdef DEBUG_BUILD
  Serial.begin(9600);
  while (!Serial)
    ;  // wait until the serial monitor is opened
#endif

  modemSerial.begin(9600);
  enableSensors();
}





// Main loop of the program
void loop() {
  // If any sensor value changed, push that new measurement to the queue
  // so it gets sent to thingspeak
  if (sensorValueChanged()) {
    SensorReading r;
    r.sensor1 = sensorValue(0);
    r.sensor2 = sensorValue(1);
    sensorReadingQueue.enqueue(r);
  }

  // Increase the number of elapsed seconds by 8. This is just a crude
  // estimation
  elapsedSeconds += 8;

  // Execute regular mode behavior
  if (mode == REGULAR) {
    // If a new regular update is due, put the current sensor values in the
    // queue and set up the following regular update.
    if (elapsedSeconds >= nextRegularUpdate) {
      // Put sensor reading to the queue
      SensorReading r;
      r.sensor1 = sensorValue(0);
      r.sensor2 = sensorValue(1);
      sensorReadingQueue.enqueue(r);

      // Set up the next regular update
      nextRegularUpdate = elapsedSeconds + REGULAR_UPDATE_INTERVAL;
    }

    // Transmit everything that is currently in the queue to thingspeak.
    // If that fails transition to WAIT_AFTER_FAIL mode and set up the
    // time when the next retry should happen
    if (!transmitQueueContent()) {
      mode = WAIT_AFTER_FAIL;
      nextRetryUpdate = elapsedSeconds + DELAY_AFTER_TRANSMISSION_FAILURE;
    }
  }

  // Execute WAIT_AFTER_FAIL mode behavior
  if (mode == WAIT_AFTER_FAIL) {
    // Check if we should already try again
    if (elapsedSeconds >= nextRetryUpdate) {
      if (transmitQueueContent()) {
        mode =
            REGULAR;  // Transmission successfull -> transition to REGULAR mode
      } else {
        mode = WAIT_AFTER_FAIL;  // Transmission not successfull -> stay in
                                 // WAIT_AFTER_FAIL mode and set up a new retry
                                 // time.
        nextRetryUpdate = elapsedSeconds + DELAY_AFTER_TRANSMISSION_FAILURE;
      }
    }
  }

  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

/// Transmits all items currently in the queue.
/// Returns true if everything was send successfully,
/// false otherwise.
bool transmitQueueContent() {
  while (!sensorReadingQueue.isEmpty()) {
    D_MSG(0, "There are still elements in the queue");
    SensorReading r = sensorReadingQueue.peek();
    if (modem.pushToThingSpeak(NUM_TRANSMISSION_RETRIES, batteryVoltage(),
                               r.sensor1, r.sensor2)) {
      sensorReadingQueue.dequeue();
    } else {
      return false;
    }
  }
  return true;
}
