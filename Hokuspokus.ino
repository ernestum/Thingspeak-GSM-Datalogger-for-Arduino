#include <SoftwareSerial.h>
#include <LowPower.h>
//#define DEBUG_BUILD
#include "Debugging.h"

#include "PowerControledThingspeakConnection.h"
// Controll pins
#define GNDCTRLPIN 12
#define VCC2TRLPIN 9
#define MODEM_RESET_PIN 4

#define NUM_TRANSMISSION_RETRIES 5
#define DELAY_AFTER_TRANSMISSION_FAILURE 60*60*5 //in seconds
#define REGULAR_UPDATE_INTERVAL 60*60*22 //in seconds

//http://api.thingspeak.com/update?api_key=YDHVCBHPKX9TOPXF&field1=BATTERY&field2=SENSOR1&field3=SENSOR2
SoftwareSerial modemSerial(8, 3); // RX, TX

PowerControledThingspeakConnection modem(modemSerial, GNDCTRLPIN, VCC2TRLPIN, MODEM_RESET_PIN);

typedef struct {
  int sensor1;
  int sensor2;
}
SensorReading;

#define QSIZE 5
template<typename T>
class Queue {
  public:
    Queue() : entry(0), exit(0), size(0) {
    }

    bool isEmpty() {
      return size == 0;
    }

    bool isFull() {
      return size == QSIZE;
    }

    void enqueue(const T e) {
      if (isFull())
        exit = next(exit);
      else
        ++size;
      array[entry] = e;
      entry = next(entry);
    }

    T dequeue() {
      if (!isEmpty()) {
        exit = next(exit);
        --size;
      }
      return array[prev(exit)];
    }


    T peek() {
      return array[exit];
    }

//    void printOut() {
//      for (int i = 0; i < QSIZE; i++) {
//        Serial.print(array[i]); Serial.print('\t');
//      }
//      Serial.println();
//      for (int i = 0; i < QSIZE; i++) {
//        char c = ' ';
//        if (i == entry) c = '^';
//        if (i == exit) c = 'v';
//        if (i == entry && i == exit) c = 'I';
//        Serial.print(c); Serial.print('\t');
//      }
//      Serial.print("size: "); Serial.print(size);
//      Serial.println();
//      Serial.println();
//    }

    uint8_t size;
    T array[QSIZE];

  private:
    uint8_t entry, exit;

    inline uint8_t next(uint8_t i) {
      return (i + 1) % QSIZE;
    }

    inline uint8_t prev(uint8_t i) {
      return i == 0 ? QSIZE - 1 : i - 1;
    }
};

Queue<SensorReading> sensorReadingQueue;


void setup()
{
#ifdef DEBUG_BUILD
  Serial.begin(9600);
  while (!Serial);
#endif

  modemSerial.begin(9600);
  //  while (!Serial) {} //Needs to be commented out when running in "headless" mode (without a serial monitor open)
  enableSensors();

}

//void queueTest() {
//  Queue<int> testQueue;
//  testQueue.enqueue(1);
//  testQueue.enqueue(2);
//  testQueue.enqueue(3);
//  testQueue.enqueue(4);
//  testQueue.enqueue(5);
//  testQueue.enqueue(6);
//  testQueue.enqueue(7);
//  testQueue.printOut();
//  Serial.println(testQueue.dequeue());
//  Serial.println(testQueue.dequeue());
//  Serial.println(testQueue.dequeue());
//
//  testQueue.printOut();
//
//  testQueue.enqueue(10);
//  testQueue.enqueue(11);
//
//  testQueue.printOut();
//  
//  Serial.println(testQueue.dequeue());
//  Serial.println(testQueue.dequeue());
//  Serial.println(testQueue.dequeue());
//
//  testQueue.printOut();
//
//  Serial.println(testQueue.dequeue());
//  Serial.println(testQueue.dequeue());
//}

unsigned long elapsedSeconds = 0;
unsigned long nextRegularUpdate = 0;
unsigned long nextRetryUpdate = 0;


#define REGULAR 0
#define WAIT_AFTER_FAIL 1

uint8_t mode = REGULAR;

void loop() {
  if (sensorValueChanged()) {
    SensorReading r;
    r.sensor1 = sensorValue(0);
    r.sensor2 = sensorValue(1);
    sensorReadingQueue.enqueue(r);
  }

  elapsedSeconds += 8;

  if (mode == REGULAR) {
    if (elapsedSeconds >= nextRegularUpdate) {
      SensorReading r;
      r.sensor1 = sensorValue(0);
      r.sensor2 = sensorValue(1);
      sensorReadingQueue.enqueue(r);
      nextRegularUpdate = elapsedSeconds + REGULAR_UPDATE_INTERVAL;
    }


    if (!transmitQueueContent()) {
      mode = WAIT_AFTER_FAIL;
      nextRetryUpdate = elapsedSeconds + DELAY_AFTER_TRANSMISSION_FAILURE;
    }
  }

  if (mode == WAIT_AFTER_FAIL) {
    if (elapsedSeconds >= nextRetryUpdate) {
      if (transmitQueueContent()) {
        mode = REGULAR;
      } else {
        mode = WAIT_AFTER_FAIL;
        nextRetryUpdate = elapsedSeconds + DELAY_AFTER_TRANSMISSION_FAILURE;
      }
    }
  }

  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  //delay(8000);

}

bool transmitQueueContent() {
  while (!sensorReadingQueue.isEmpty()) {
    D_MSG(0, "There are still elements in the queue");
    SensorReading r = sensorReadingQueue.peek();
    if (modem.pushToThingSpeak(NUM_TRANSMISSION_RETRIES,
                               batteryVoltage(),
                               r.sensor1,
                               r.sensor2)) {
      sensorReadingQueue.dequeue();
    } else {
      return false;
    }
  }
  return true;
}
