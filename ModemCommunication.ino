boolean tryPushToThingSpeak(float bat, int sensor1, int sensor2) {
  debug("Disabeling echo!");
  int firstCommandTries = 5;
  bool fistCommandSuccess = false;
  while(firstCommandTries > 0 && fistCommandSuccess == false) {
    fistCommandSuccess = sendCommand("ATE0", "OK\r\n", 1000);
    --firstCommandTries;
  }
  if(!fistCommandSuccess)
    return false;

  debug("Clone any previous connections");
  sendCommand("AT+QICLOSE", "OK\r\n", 1000);

  debug("Setting the apn access details");
  sendCommand("AT+QIREGAPP=\"web.be\",\"web\",\"web\"", "OK\r\n", 1000);

  debug("Telling him to print out the incomming tcp data");
  if(!sendCommand("AT+QINDI=0", "OK\r\n", 1000)) {
    debug("This Failed");
    return false;
  }

  debug("Opening a connection to thingspeak");
  if (!sendCommand("AT+QIOPEN=\"TCP\",\"184.106.153.149\",80", "OK\r\nCONNECT OK\r\n", 20000)) {
    debug("Could not open TCP connection");
    return false;
  }

  debug("Sending the right http request");
  if(!sendCommand("AT+QISEND", "> ", 1000)) {
    debug("Getting no input prompt to send TCP data!");
    return false;
  }
    
  Serial1.print("GET /update?api_key=YDHVCBHPKX9TOPXF&field1=");
  Serial1.print(bat);
  Serial1.print("&field2=");
  Serial1.print(sensor1);
  Serial1.print("&field3=");
  Serial1.print(sensor2);
  Serial1.print("&headers=false");
  Serial1.println(" HTTP/1.0\n\n\x1A"); Serial1.flush();
  if(!waitFor("SEND OK\r\n", 20000)) {
    debug("Sending was not successful!");
    return false;
  } else debug("YEA SENT DATA OUT!!!");
  if(!readUntil("CLOSED\r\n", 20000))
    debug("Somehow we did not read CLOSED!");
  //Serial1.println("GET /update?api_key=YDHVCBHPKX9TOPXF&field1=42&field2=5&field3=5&headers=false HTTP/1.0\n\n\x1A");
  //Serial1.println("GET / HTTP/1.0\n\n\x1A");
  return true;
}

boolean sendCommand(char* command, char* expectedAnswer, unsigned int timeout) {
  Serial1.println(command);
  Serial1.flush();
  if(!waitFor(expectedAnswer, timeout)) {
#ifdef DEBUG
      Serial.print("Timeout or wrong answer while executing '");
      Serial.print(command);
      Serial.print("' and waiting for the answer '");
      Serial.print(expectedAnswer);
      Serial.println('\'');
#endif
    return false;
  }
  return true;
}

/**
 * Reads from Serial1 and checks if the incoming bytes match an expected answer.
 * Leading carriage returns and newlines are skipped. As soon as a byte dos not
 * match the expected answer it skips to the first newline.
 * Returns true if the full answer was matched. False otherwise
 */
boolean waitFor(char* answer, unsigned int timeout) {
  unsigned long start = millis();
  while (*answer != '\0') {
    if (!waitForNextChar(timeout) || millis() - start >= timeout) {
#ifdef DEBUG
      Serial.print("Timeout while waiting for: ");
      Serial.println(answer);
#endif
      return false;
    }
    char inchar = Serial1.read();
    if (inchar == *answer) {
#ifdef DEBUG
      printCharDetail(inchar);
#endif
      ++answer;
    } else if (inchar == '\r' || inchar == '\n') {
      continue;
    }
    else {
#ifdef DEBUG
      Serial.print("XXX");
#endif
      readUntil('\n', timeout);
      return false;
    }
  }
  return true;
}

/**
 * Reads from Serial1 until a given char is read
 */
boolean readUntil(char c, unsigned int timeout) {
  char inchar = c - 1;
  unsigned long start = millis();
  while (inchar != c) {
    if (!waitForNextChar(timeout)) {
#ifdef DEBUG
      Serial.print("Timeout while reading until ");
      printCharDetail(c);
      Serial.println();
#endif   
      return false;
    }
    inchar = Serial1.read();
    if (millis() - start >= timeout)
      return false;
#ifdef DEBUG
    printCharDetail(inchar);
#endif
  }
  return true;
}

boolean readUntil(char* answer, unsigned int timeout) {
  char* startChar = answer;
  unsigned long start = millis();
  while(*answer != '\0') {
    if(!waitForNextChar(timeout)) {
     return false; 
    }
    char inchar = Serial1.read();
    if(inchar == *answer)
      ++answer;
    else
      answer = startChar;
  }
  return true;
}

/**
 * Actively waits for a new char to be available on Serial1
 */
boolean waitForNextChar(unsigned int timeout) {
  unsigned long start = millis();
  while (Serial1.available() == 0) {
    if (millis() - start >= timeout) {
#ifdef DEBUG
      Serial.println("Timeout while waiting for next char");
#endif
      return false;
    }
  }
  return true;
}

//#ifdef DEBUG
void printCharDetail(char incoming_char) {
  Serial.print((int)incoming_char);
  Serial.print('[');
  Serial.print((char)incoming_char);
  Serial.print(']');
  Serial.print(' ');
}
//#endif
