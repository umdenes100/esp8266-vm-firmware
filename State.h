#pragma once
#include <Arduino.h>

// Serial opcodes (legacy)
#define OP_BEGIN            0x1
#define OP_PRINT            0x2
#define OP_CHECK            0x3
#define OP_MISSION          0x4
#define OP_ML_PREDICTION    0x5   // kept for compatibility (unused here)
#define OP_ML_CAPTURE       0x6   // kept for compatibility (unused here)
#define OP_IS_CONNECTED     0x7
#define OP_PRED             0x8   // kept for compatibility (unused here)

// End-of-message flush bytes from Arduino to ESP
static const uint8_t FLUSH_SEQUENCE[4] = {0xFF,0xFE,0xFD,0xFC};

struct State {
  // Team/session
  String  teamName;
  uint8_t teamType = 0;
  int     arucoId  = -1;
  int     roomNum  = -1;

  // Latest AR marker data (from server)
  volatile double aruco_x = 0;
  volatile double aruco_y = 0;
  volatile double aruco_theta = 0;
  volatile bool   aruco_visible = false;
  volatile bool   newData = false;
  volatile bool   arucoConfirmed = false;

  // Connection flags
  volatile bool wsConnected = false;
};
