#ifndef THINGSPEAKCONNECTION_H
#define THINGSPEAKCONNECTION_H
#include "Debugging.h"
#include "Timeout.h"

#define THINGSPEAK_CHANNEL_KEY "YDHVCBHPKX9TOPXF"
#define GPRS_APN "web.be"
#define GPRS_USER "web"
#define GPRS_PASSWORD "web"

/// Timeouts are in milliseconds!
/// To be used for operations for which we expect a fast response like setting
/// parameters on the modem etc.
#define FAST_TIMEOUT 2000

/// To be used for modem operations which might take a little longer like gprs
/// network operations or powering up the modem
#define SLOW_TIMEOUT 60000

#define NUM_TCP_CONNECTION_RETRIES 5

/// A class that sends AT commands to a GPRS modem which make it send data to a
/// thingspeak channel. It was written and tested with a Quectel M10 modem.
class ThingspeakConnection {
 public:
  ThingspeakConnection(Stream& communicationStream)
      : serial(&communicationStream) {}

  /// Tries to push the battery status and two sensor values to thingspeak.
  /// Returns true if it was successfull (HTTP 200 and nonzero answer from
  /// thingspeak), false if something went wrong (even after waiting long and
  /// trying some things a couple of times).
  boolean tryPushToThingSpeak(float bat, int sensor1, int sensor2) {
    D_MSG(2, "Now try to push data");
    D_MSG(2, "Disabeling echo!");
    // Firs we try for at least 5 times to send a simple command to the modem
    // and check if it responds as expected
    int firstCommandTries = 5;
    bool fistCommandSuccess = false;
    while (firstCommandTries > 0 && fistCommandSuccess == false) {
      fistCommandSuccess = sendCommand("ATE0", "OK\r\n", FAST_TIMEOUT);
      --firstCommandTries;
    }
    if (!fistCommandSuccess) return false;

    // Here we set up and prepeare everything for the new TCP connection to be
    // opened
    D_MSG(2, "Close any previous connections");
    sendCommand("AT+QICLOSE", "OK\r\n", FAST_TIMEOUT);

    D_MSG(2, "Setting the apn access details");
    sendCommand("AT+QIREGAPP=\"" GPRS_APN "\",\"" GPRS_USER
                "\",\"" GPRS_PASSWORD "\"",
                "OK\r\n", FAST_TIMEOUT);

    D_MSG(2,
          "Telling him to print out the incoming tcp data directly to the "
          "serial interface");
    if (!sendCommand("AT+QINDI=0", "OK\r\n", FAST_TIMEOUT)) {
      D_MSG(2, "This Failed");
      return false;
    }

    D_MSG(2, "Waiting for the IP stack to be ready");
    if (!waitForTCPStack(SLOW_TIMEOUT)) {
      D_MSG(2, "No Ready IP stack?");
      return false;
    }

    D_MSG(2, "Opening a connection to thingspeak");
    bool tcpConnectSucess = false;
    for (int i = 0; i < NUM_TCP_CONNECTION_RETRIES && tcpConnectSucess == false;
         i++) {
      // Wait longer and longer after each new retry
      delay(FAST_TIMEOUT * i);
      // Close a possibly already established connection
      sendCommand("AT+QICLOSE", "OK\r\n", FAST_TIMEOUT);
      // Here we increase the timeout with each new retry
      tcpConnectSucess = sendCommand("AT+QIOPEN=\"TCP\",\"184.106.153.149\",80",
                                     "OK\r\nCONNECT OK\r\n", SLOW_TIMEOUT * i);
    }
    if (!tcpConnectSucess) {
      D_MSG(2, "Could not open TCP connection");
      return false;
    }
    D_MSG(2, "Successfully opened TCP connection!");

    D_MSG(2, "Sending the http request");
    if (!sendCommand("AT+QISEND", "> ", FAST_TIMEOUT)) {
      D_MSG(2, "Getting no input prompt to send TCP data!");
      return false;
    }

    /// **********************************
    /// Here we do the actual HTTP request
    /// **********************************
    serial->print("GET /update?api_key=");
    serial->print(THINGSPEAK_CHANNEL_KEY);
    serial->print(" &field1=");
    serial->print(bat);
    serial->print("&field2=");
    serial->print(sensor1);
    serial->print("&field3=");
    serial->print(sensor2);
    serial->print("&headers=false");
    serial->println(" HTTP/1.0\n\n\x1A");
    serial->flush();
    /// **********************************

    if (!waitFor("SEND OK\r\n", SLOW_TIMEOUT)) {
      D_MSG(2, "Sending was not successful!");
      return false;
    } else
      D_MSG(2, "TCP Data Sent");

    if (!readUntil("Status: 200 OK", SLOW_TIMEOUT)) {
      D_MSG(2, "HTML Status is not 200, there is some error!");
      return false;
    }
    readUntil("\r\n\r\n", SLOW_TIMEOUT);  // Read until that empty line right
                                          // after the http header
    int responseNr = serial->parseInt();
    if (responseNr == 0) {
      D_MSG(2,
            "The running counter from thingspeak was 0! We did not "
            "successfully send the data!");
      return false;
    } else {
      D_MSG(2, "Response was");
      D_MSG(4, responseNr);
    }

    if (!readUntil("CLOSED\r\n", SLOW_TIMEOUT))
      D_MSG(
          2,
          "Somehow we did not read CLOSED! We still assume the data was sent");

    return true;
  }

  /// Sends a command and waits for an expected answer by the modem as long as
  /// the timeout did not elapse. Returns true, if the answer was actually
  /// returned in time, false it the timeout elapsed or the answer was a
  /// different one.
  boolean sendCommand(char* command, char* expectedAnswer,
                      unsigned int timeout) {
    D_MSG(3, command);
    serial->println(command);
    serial->flush();
    if (!waitFor(expectedAnswer, timeout)) {
      D_MSG(4, "Timeout or wrong answer while executing command");
      return false;
    }
    return true;
  }

  /// To be called repeatedly in the main loop in order to establish a bridge
  /// between the main Serial interface and the serial interface of the modem.
  void modemSerialBridgeLoop() {
    char incoming_char;
    if (serial->available() > 0) {
      incoming_char = serial->read();
      Serial.print(incoming_char);
    }

    if (Serial.available() > 0) {
      incoming_char = Serial.read();
      serial->print(incoming_char);
    }
  }

  /// Reads from the serial inteface and checks if the incoming bytes match an
  /// expected answer. Leading carriage returns and newlines are skipped. As
  /// soon as a byte does not match the expected answer, it skips to the first
  /// newline. Returns true if the full answer was matched, false otherwise.
  boolean waitFor(char* answer, unsigned int timeout) {
    Timeout t(timeout);
    while (*answer != '\0') {
      if (!waitForNextChar(timeout) || t.elapsed()) {
        D_MSG(4, "Timeout while waiting for: ");
        D_MSG(5, answer);
        return false;
      }
      char inchar = serial->read();
      if (inchar == *answer) {
        ++answer;
      } else if (inchar == '\r' || inchar == '\n') {
        continue;
      } else {
        D_MSG(4, "Read something else than");
        D_MSG(5, answer);
        readUntil('\n', timeout);
        return false;
      }
    }
    return true;
  }

 private:
  /// The serial interface to be used to communicate with the modem
  Stream* serial;

  /// Waits until the TCP stack of the modem is ready. For this the state of the
  /// TCP stack is constantly checked using AT+QISTAT in 50ms intervals. If the
  /// stack is not ready before the timeout elapsed we give up and return false,
  /// otherwise true.
  boolean waitForTCPStack(unsigned int timeout) {
    Timeout t(timeout);
    while (
        !sendCommand("AT+QISTAT", "OK\r\nSTATE: IP START\r\n", FAST_TIMEOUT)) {
      if (t.elapsed()) return false;
      delay(50);
    }
    return true;
  }

  /// Reads from the serial interface until a certain character sequence is
  /// found or the timeout elapsed. Returns true if the character sequence was
  /// found, false if the timout elapsed.
  boolean readUntil(char* answer, unsigned int timeout) {
    char* startChar = answer;
    Timeout t(timeout);
    while (*answer != '\0') {
      if (!waitForNextChar(timeout) || t.elapsed()) {
        return false;
      }
      char inchar = serial->read();
      if (inchar == *answer)
        ++answer;
      else
        answer = startChar;
    }
    return true;
  }

  /// Reads from the serial interface until a given char c is read or the
  /// timeout elapsed. Returns true if we actually found the char, false if the
  /// timeout elapsed.
  boolean readUntil(char c, unsigned int timeout) {
    char inchar = c - 1;
    Timeout t(timeout);
    while (inchar != c) {
      if (!waitForNextChar(timeout)) {
        D_MSG(4, "Timeout while reading until");
        D_MSG(5, c);
        return false;
      }
      inchar = serial->read();
      if (t.elapsed()) return false;
    }
    return true;
  }

  /// Actively (blocking) waits for a new char to be available on the serial
  /// interface. Returns true if there actually was a next char, false if we got
  /// impatient and cancelled waiting because of the timeout
  boolean waitForNextChar(unsigned int timeout) {
    Timeout t(timeout);
    while (serial->available() == 0) {
      if (t.elapsed()) {
        D_MSG(6, "Timeout waiting for next char");
        return false;
      }
    }
    return true;
  }
};

#endif
