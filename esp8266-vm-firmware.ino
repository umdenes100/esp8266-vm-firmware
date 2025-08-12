#include "Config.h"
#include "Utils_Log.h"
#include "State.h"
#include "Net_Wifi.h"
#include "Net_WebSocketClient.h"
#include "SerialProtocol.h"

State g;
WebSocketLink ws(g);
SerialProtocol sp(g, ws);

void setup(){
  // Use HW UART (RX0/TX0) for Uno link on ESP8266 TXD/RXD pins.
  Serial.begin(ARD_BAUD);
  delay(20);
#ifdef DEBUG
  ELog::info("DEBUG ENABLED");
#endif

  sp.beginSerial();     // sets internal state, starts ready-heartbeat immediately
  Net::wifiBegin();     // MAC spoof inside per Config.h
  ws.begin();           // connect websocket (non-blocking poll in loop)
}

void loop(){
  sp.poll();            // handle serial (incl. ready-heartbeat + OP_* handling)
  ws.poll();            // websocket client loop
  yield();
}
