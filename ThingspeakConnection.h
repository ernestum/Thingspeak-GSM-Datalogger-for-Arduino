#include <Arduino.h>

#ifndef THINGSPEAKCONNECTION_H
#define THINGSPEAKCONNECTION_H
#include "Debugging.h"
#include "Timeout.h"

//Timeouts in milliseconds
#define FAST_TIMEOUT 2000 //To be used for operations for which we expect a fast response like setting parameters on the modem etc.
#define SLOW_TIMEOUT 60000 //To be used for modem operations which might take a little longer like gprs network operations or powering up the modem

#define NUM_TCP_CONNECTION_RETRIES 5

#define GPRS_APN "web.be"
#define GPRS_USER "web"
#define GPRS_PASSWORD "web"

//#ifdef DEBUG
void printCharDetail(char incoming_char) {
  Serial.print((int)incoming_char);
  Serial.print('[');
  Serial.print((char)incoming_char);
  Serial.print(']');
  Serial.print(' ');
}
//#endif


class ThingspeakConnection {
  public:
    ThingspeakConnection(Stream &communicationStream) :
      serial(&communicationStream), lastPackageSent(0)
    {

    }

    boolean tryPushToThingSpeak(float bat, int sensor1, int sensor2) {
      debug("Trypush of pure");
      debug("Disabeling echo!");
      int firstCommandTries = 5;
      bool fistCommandSuccess = false;
      while (firstCommandTries > 0 && fistCommandSuccess == false) {
        fistCommandSuccess = sendCommand("ATE0", "OK\r\n", FAST_TIMEOUT);
        --firstCommandTries;
      }
      if (!fistCommandSuccess)
        return false;

      debug("Close any previous connections");
      sendCommand("AT+QICLOSE", "OK\r\n", FAST_TIMEOUT);

      debug("Setting the apn access details");
      sendCommand("AT+QIREGAPP=\"" GPRS_APN "\",\"" GPRS_USER "\",\"" GPRS_PASSWORD "\"", "OK\r\n", FAST_TIMEOUT);
      //      sendCommand("AT+QIREGAPP=\"web.be\",\"web\",\"web\"", "OK\r\n", FAST_TIMEOUT);

      debug("Telling him to print out the incomming tcp data");
      if (!sendCommand("AT+QINDI=0", "OK\r\n", FAST_TIMEOUT)) {
        debug("This Failed");
        return false;
      }
      debug("Waiting for the IP STACK to be ready");
      if (!waitForTCPStack(SLOW_TIMEOUT)) {
        debug("No Ready IP Stack?");
        return false;
      }

      debug("Opening a connection to thingspeak");
      bool tcpConnectSucess = false;
      for (int i = 0; i < NUM_TCP_CONNECTION_RETRIES && tcpConnectSucess == false; i++) {
        delay(FAST_TIMEOUT*i); //wait quite long between tcp connection attempts
        sendCommand("AT+QICLOSE", "OK\r\n", FAST_TIMEOUT);
        tcpConnectSucess = sendCommand("AT+QIOPEN=\"TCP\",\"184.106.153.149\",80", "OK\r\nCONNECT OK\r\n", SLOW_TIMEOUT*i);
      }
      if (!tcpConnectSucess) {
        debug("Could not open TCP connection");
        return false;
      }

      debug("Sending the right http request");
      if (!sendCommand("AT+QISEND", "> ", FAST_TIMEOUT)) {
        debug("Getting no input prompt to send TCP data!");
        return false;
      }

      /// **********************************
      /// Here we do the actual HTTP request
      /// **********************************
      serial->print("GET /update?api_key=YDHVCBHPKX9TOPXF&field1=");
      serial->print(bat);
      serial->print("&field2=");
      serial->print(sensor1);
      serial->print("&field3=");
      serial->print(sensor2);
      serial->print("&headers=false");
      serial->println(" HTTP/1.0\n\n\x1A"); serial->flush();
      /// **********************************

      if (!waitFor("SEND OK\r\n", SLOW_TIMEOUT)) {
        debug("Sending was not successful!");
        return false;
      } else debug("YEA SENT DATA OUT!!!");
      if (!readUntil("Status: 200 OK", SLOW_TIMEOUT)) {
        debug("HTML Status is not 200, there is some error!");
        return false;
      }
      readUntil("\r\n\r\n", SLOW_TIMEOUT);//Read until that empty line right after the http header
      int responseNr = serial->parseInt();
      if (responseNr == 0) {
        debug("The running counter from thingspeak was 0! We did not successfully send the data!");
        return false;
      } else {
        debug("Response was"); Serial.println(responseNr);
      }

      if (!readUntil("CLOSED\r\n", SLOW_TIMEOUT))
        debug("Somehow we did not read CLOSED!");

      lastPackageSent = millis(); //TODO maybe replace this with a different timer because millis might be broken with the watchdog
      return true;
    }

    boolean sendCommand(char* command, char* expectedAnswer, unsigned int timeout) {
      debug(command);
      serial->println(command);
      serial->flush();
      if (!waitFor(expectedAnswer, timeout)) {
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

    void modemSerialBridgeLoop() {
      char incoming_char;
      if (serial->available() > 0)
      {
        incoming_char = serial->read();
        Serial.print(incoming_char);
      }

      if (Serial.available() > 0)
      {
        incoming_char = Serial.read();
        serial->print(incoming_char);
      }
    }

    /**
    * Reads from Serial1 and checks if the incoming bytes match an expected answer.
    * Leading carriage returns and newlines are skipped. As soon as a byte dos not
    * match the expected answer it skips to the first newline.
    * Returns true if the full answer was matched. False otherwise
    */
    boolean waitFor(char* answer, unsigned int timeout) {
      Timeout t(timeout);
      while (*answer != '\0') {
        if (!waitForNextChar(timeout) || t.elapsed()) {
#ifdef DEBUG
          Serial.print("Timeout while waiting for: ");
          Serial.println(answer);
#endif
          return false;
        }
        char inchar = serial->read();
        if (inchar == *answer) {
#ifdef DEBUG
          Serial.print(inchar);
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

  private:
    Stream* serial;
    unsigned long lastPackageSent;

    boolean waitForTCPStack(unsigned int timeout) {
      Timeout t(timeout);
      while (!sendCommand("AT+QISTAT", "OK\r\nSTATE: IP START\r\n", FAST_TIMEOUT)) {
        if (t.elapsed())
          return false;
        delay(50);
      }
      return true;
    }



    /**
     * Reads from Serial1 until a given char is read
     */
    boolean readUntil(char c, unsigned int timeout) {
      char inchar = c - 1;
      Timeout t(timeout);
      while (inchar != c) {
        if (!waitForNextChar(timeout)) {
#ifdef DEBUG
          Serial.print("Timeout while reading until ");
          Serial.print(inchar);
          Serial.println();
#endif
          return false;
        }
        inchar = serial->read();
        if (t.elapsed())
          return false;
#ifdef DEBUG
        Serial.print(inchar);
#endif
      }
      return true;
    }

    boolean readUntil(char* answer, unsigned int timeout) {
      char* startChar = answer;
      Timeout t(timeout);
      while (*answer != '\0') {
        if (!waitForNextChar(timeout) || t.elapsed()) {
          return false;
        }
        char inchar = serial->read();
#ifdef DEBUG
        Serial.print(inchar);
#endif
        if (inchar == *answer)
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
      Timeout t(timeout);
      while (serial->available() == 0) {
        if (t.elapsed()) {
#ifdef DEBUG
          Serial.println("Timeout while waiting for next char");
#endif
          return false;
        }
      }
      return true;
    }
};

#endif
