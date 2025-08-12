#pragma once
#include <Arduino.h>
#include "State.h"
#include "Net_WebSocketClient.h"
#include "Config.h"
#include "Utils_Log.h"

// Hardware UART only (matches your wiring):
// Uno SoftwareSerial: pin 3 (TX) -> ESP RXD (GPIO3), pin 2 (RX) <- ESP TXD (GPIO1)
// GNDs must be common; Uno TX must be level-shifted to 3.3V before ESP RX.

class SerialProtocol {
public:
  SerialProtocol(State& stateRef, WebSocketLink& wsRef)
  : s_(stateRef), ws_(wsRef) {}

  void beginSerial() {
    // .ino already did Serial.begin(ARD_BAUD)
    // Start sending "ready" 0x00 heartbeats so Uno's state() sees something immediately.
    lastReadyMs_ = millis();
    haveBegun_ = false;
#ifdef DEBUG
    ELog::info(String("HW Serial listening @ ") + String(ARD_BAUD));
#endif
  }

  void poll() {
    // 1) Ready heartbeat until OP_BEGIN arrives:
    if (!haveBegun_) {
      unsigned long now = millis();
      if (now - lastReadyMs_ >= READY_HEARTBEAT_MS) {
        Serial.write((uint8_t)0x00);   // "not connected yet" state byte
        Serial.flush();
        lastReadyMs_ = now;
      }
    }

    // 2) Parse incoming bytes from Uno
    while (Serial.available()){
      uint8_t b = Serial.read();
      if (idx_ < sizeof(buf_)) buf_[idx_++] = b;

      // End-of-message (flush sequence)
      if (idx_ >= 4 &&
          buf_[idx_-4] == FLUSH_SEQUENCE[0] &&
          buf_[idx_-3] == FLUSH_SEQUENCE[1] &&
          buf_[idx_-2] == FLUSH_SEQUENCE[2] &&
          buf_[idx_-1] == FLUSH_SEQUENCE[3]) {
        handleCommand();
        idx_ = 0;
      }

      // Immediate single-byte ops:
      if (idx_ == 1 && buf_[0] == OP_CHECK) {
        idx_ = 0; handleOpCheck();
      }
      if (idx_ == 1 && buf_[0] == OP_IS_CONNECTED) {
        idx_ = 0; Serial.write(ws_.isConnected() ? (uint8_t)0x01 : (uint8_t)0x00);
      }
    }
  }

private:
  void handleCommand() {
    if (idx_ < 1) return;
    uint8_t op = buf_[0];

    if (op == OP_BEGIN) {
      // [OP_BEGIN][teamType:1][markerId:2][room:2][teamName:char*][FLUSH]
      if (idx_ < 8) return;
      s_.teamType = buf_[1];
      s_.arucoId  = ((int)buf_[2] << 8) | buf_[3];
      s_.roomNum  = ((int)buf_[4] << 8) | buf_[5];
      s_.teamName = String((char*)&buf_[6]);
      ws_.sendBegin(s_.teamName, s_.teamType, s_.arucoId, s_.roomNum);

      // stop ready-heartbeat once we've begun
      haveBegun_ = true;
    }
    else if (op == OP_MISSION) {
      // [OP_MISSION][type:1][message:char*][FLUSH]
      if (idx_ < 3) return;
      uint8_t type = buf_[1];
      char* msg = (char*)&buf_[2];
      ws_.sendMission(s_.teamName, type, msg);
    }
    else if (op == OP_PRINT) {
      // [OP_PRINT][message:char*][FLUSH]
      if (idx_ < 2) return;
      char* msg = (char*)&buf_[1];
      ws_.sendPrint(s_.teamName, msg);
    }
    // other opcodes ignored (ML, etc.)
  }

  void handleOpCheck() {
    if (s_.newData) {
      s_.newData = false;
      if (s_.aruco_visible) {
        // 0x02 + y(1B, uint8, *100) + x(2B LE, uint16, *100) + theta(2B LE, int16, *100)
        Serial.write((uint8_t)0x02);

        long yScaled = (long)(s_.aruco_y * 100.0 + (s_.aruco_y >= 0 ? 0.5 : -0.5));
        if (yScaled < 0) yScaled = 0; if (yScaled > 255) yScaled = 255;
        uint8_t yByte = (uint8_t)yScaled;
        Serial.write(yByte);

        long xScaled = (long)(s_.aruco_x * 100.0 + (s_.aruco_x >= 0 ? 0.5 : -0.5));
        if (xScaled < 0) xScaled = 0; if (xScaled > 65535) xScaled = 65535;
        uint16_t xVal = (uint16_t)xScaled;
        Serial.write((uint8_t*)&xVal, 2);

        long tScaled = (long)(s_.aruco_theta * 100.0 + (s_.aruco_theta >= 0 ? 0.5 : -0.5));
        if (tScaled < -32768) tScaled = -32768; if (tScaled > 32767) tScaled = 32767;
        int16_t tVal = (int16_t)tScaled;
        Serial.write((uint8_t*)&tVal, 2);

        Serial.flush();
      } else {
        Serial.write((uint8_t)0x01); // marker not visible
        Serial.flush();
      }
    } else {
      Serial.write((uint8_t)0x00); // no update
      Serial.flush();
    }
  }

  // Members
  uint8_t      buf_[500];
  size_t       idx_ = 0;
  State&       s_;
  WebSocketLink& ws_;
  bool         haveBegun_ = false;
  unsigned long lastReadyMs_ = 0;
  static constexpr unsigned long READY_HEARTBEAT_MS = 20; // every 20ms until OP_BEGIN
};
