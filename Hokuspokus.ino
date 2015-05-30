
// Controll pins
#define GNDCTRLPIN 12
#define VCC2TRLPIN 9
#define MODEM_RESET_PIN 4

// Sensor pins and interupts
#define SENS1INTER 1
#define SENS2INTER 4
#define SENS1PIN 2
#define SENS2PIN 7
#define BATTSENSPIN A5

#define DEBUG

//http://api.thingspeak.com/update?api_key=YDHVCBHPKX9TOPXF&field1=BATTERY&field2=SENSOR1&field3=SENSOR2


inline void debug(char* message) {
#ifdef DEBUG
  Serial.print("\t->"); Serial.println(message); Serial.flush();
#endif  
}

void setup()
{
  float bat = 20, sensor1 = 20, sensor2 = 20;
  Serial.begin(9600);
  while (!Serial) {}
  Serial1.begin(9600);
  Serial.print("Starting modem communication...");
  delay(2000);
  Serial.print("OK\nIntroduce your AT commands:\n");


  sendData(0, 0, 0);

  Serial.println("Finished Listening");

  // configure some pins
  pinMode(GNDCTRLPIN, OUTPUT);
  pinMode(VCC2TRLPIN, OUTPUT);
  pinMode(MODEM_RESET_PIN, OUTPUT);
  pinMode(SENS1PIN, INPUT_PULLUP);
  pinMode(SENS2PIN, INPUT_PULLUP);


  delay(500);

}

void readAllFromSerial1(int timeout) {
  unsigned long start = millis();
  while (millis() - start < timeout)
    while (Serial1.available() > 0)
      Serial.print((char)Serial1.read());
}

void sendTestData() {
  Serial.println("Sending some test data");
  sendData(random(0, 50), millis() / 1000, micros() / 1000);
}


void loop() {

  //    Serial.print("Waiting for ms "); Serial.println(tenMinutes);
  //    delay(tenMinutes);
  //  manualModemControl();
  //  int bat = batteryVoltage();
  //  Serial.print(sensorValue(0)); Serial.print('\t'); Serial.print(sensorValue(1)); Serial.print('\t'); Serial.println(bat);
  //  for(;bat >0; bat -= 100)
  //    Serial.print('-');

  //  Serial.println();
  //  delay(100);
  //  enableModem();
  //  delay(300);
  //  //sendTestData();
  //  sendData(batteryVoltage(), sensorValue(0), sensorValue(1));
  //  disableModem();
  //  Serial.println("Some Testdata sent!");
  //  delay(30000);

  char incoming_char;
  if (Serial1.available() > 0)
  {
    incoming_char = Serial1.read();
    printCharDetail(incoming_char);
    //    if (incoming_char == 10)
    //      Serial.println();
    //    else
    //      Serial.print(incoming_char);
  }

  if (Serial.available() > 0)
  {
    incoming_char = Serial.read();
    Serial1.print(incoming_char);
  }
}



