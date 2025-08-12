#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include "Config.h"
#include "Utils_Log.h"
#include "State.h"

using namespace websockets;

class WebSocketLink {
public:
  explicit WebSocketLink(State& st): s(st) {}

  void begin(){
    client.onMessage([this](WebsocketsMessage msg){ onMessage(msg); });
    client.onEvent([this](WebsocketsEvent ev, String data){ onEvent(ev, data); });

    if (!client.connect(WS_URL)){
      ELog::error("WebSocket connect failed; restarting…");
      delay(1000);
      ESP.restart();
    }
    s.wsConnected = client.available();
  #ifdef DEBUG
    ELog::info("WebSocket connected");
  #endif
  }

  void poll(){
    client.poll();
    s.wsConnected = client.available();
  }

  bool sendJson(const String& text){
    return client.send(text);
  }

  bool isConnected() const {
    return s.wsConnected;
  }

  // --- Outbound messages ---
  void sendBegin(const String& teamName, uint8_t teamType, int arucoId, int roomNum){
    StaticJsonDocument<JSON_DOC_SIZE> doc;
    doc["op"] = "begin";
    doc["teamName"] = teamName;
    doc["aruco"] = arucoId;
    doc["teamType"] = teamType;
    doc["room"] = roomNum;
    String out;
    serializeJson(doc, out);
    sendJson(out);
  }

  void sendMission(const String& teamName, uint8_t type, const char* message){
    StaticJsonDocument<JSON_DOC_SIZE> doc;
    doc["op"] = "mission";
    doc["teamName"] = teamName;
    doc["type"] = (int)type;
    doc["message"] = message;
    String out;
    serializeJson(doc, out);
    sendJson(out);
  }

  void sendPrint(const String& teamName, const char* message){
    StaticJsonDocument<JSON_DOC_SIZE> doc;
    doc["op"] = "print";
    doc["teamName"] = teamName;
    doc["message"] = message;
    String out;
    serializeJson(doc, out);
    sendJson(out);
  }

private:
  void onEvent(WebsocketsEvent event, String){
    if (event == WebsocketsEvent::ConnectionOpened){
      s.wsConnected = true;
    } else if (event == WebsocketsEvent::ConnectionClosed){
      s.wsConnected = false;
      delay(1000);
      ESP.restart();
    }
  }

  void onMessage(WebsocketsMessage message){
    StaticJsonDocument<300> doc;
    DeserializationError err = deserializeJson(doc, message.data());
    if (err) return;

    const char* op = doc["op"] | "";
    if (strcmp(op, "aruco") == 0){
      s.aruco_visible = doc["aruco"]["visible"] | s.aruco_visible;
      s.aruco_x       = doc["aruco"]["x"]       | s.aruco_x;
      s.aruco_y       = doc["aruco"]["y"]       | s.aruco_y;
      s.aruco_theta   = doc["aruco"]["theta"]   | s.aruco_theta;
      s.newData = true;
    } else if (strcmp(op, "info") == 0){
      const char* loc = doc["mission_loc"] | "bottom";
      const float x = 0.55f;
      const float y = (strcmp(loc,"bottom")==0) ? 0.55f : 1.45f;
      const float theta = 0.0f;
      sendArucoFloatsToArduino(x,y,theta);
    } else if (strcmp(op, "aruco_confirm") == 0){
      s.arucoConfirmed = true;
    }
  }

  void sendArucoFloatsToArduino(float x, float y, float theta){
    union { float f; uint8_t b[4]; } f;
    // We send these over HW Serial; Uno isn't expecting these unless it's wired there.
    // (This path mirrors the old firmware behavior.)
    Serial.write(0x05); Serial.flush();
    f.f = x;     Serial.write(f.b, 4); Serial.flush();
    f.f = y;     Serial.write(f.b, 4); Serial.flush();
    f.f = theta; Serial.write(f.b, 4); Serial.flush();
  }

  State& s;
  WebsocketsClient client;
};
