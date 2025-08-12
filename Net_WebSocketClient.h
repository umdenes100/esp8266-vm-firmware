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
      delay(1000);
      ESP.restart();
    }
    s.wsConnected = client.available();
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

  // --- Outbound messages (legacy field order) ---
  void sendBegin(const String& teamName, uint8_t teamType, int arucoId, int roomNum){
    StaticJsonDocument<JSON_DOC_SIZE> doc;
    doc["op"] = "begin";
    doc["teamName"] = teamName;
    doc["aruco"] = arucoId;
    doc["teamType"] = teamType;
    doc["room"] = roomNum;
    String out; serializeJson(doc, out);
    sendJson(out);
  }

  void sendMission(const String& teamName, uint8_t type, const char* message){
    StaticJsonDocument<JSON_DOC_SIZE> doc;
    doc["op"] = "mission";
    doc["teamName"] = teamName;
    doc["type"] = (int)type;
    doc["message"] = message;
    String out; serializeJson(doc, out);
    sendJson(out);
  }

  void sendPrint(const String& teamName, const char* message){
    StaticJsonDocument<JSON_DOC_SIZE> doc;
    doc["op"] = "print";
    doc["teamName"] = teamName;
    doc["message"] = message;
    String out; serializeJson(doc, out);
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

  // Parse exactly: {"op":"aruco","aruco":{"visible":bool,"x":float,"y":float,"theta":float}}
  // Fallback: if visible==false but x/y/theta are valid (!=-1), treat as visible.
  void onMessage(WebsocketsMessage message){
    StaticJsonDocument<384> doc;
    DeserializationError err = deserializeJson(doc, message.data());
    if (err) return;

    const char* op = doc["op"] | "";
    if (strcmp(op, "aruco") == 0){
      JsonObjectConst a = doc["aruco"].as<JsonObjectConst>();
      if (a.isNull()) return;

      bool   vis   = a["visible"] | s.aruco_visible;
      double x     = a["x"]       | s.aruco_x;
      double y     = a["y"]       | s.aruco_y;
      double theta = a["theta"]   | s.aruco_theta;

      bool coordsLookValid = (x != -1.0) && (y != -1.0) && (theta != -1.0);
      if (!vis && coordsLookValid) vis = true;

      bool changed = (vis != s.aruco_visible) ||
                     (x   != s.aruco_x)       ||
                     (y   != s.aruco_y)       ||
                     (theta != s.aruco_theta);

      s.aruco_visible = vis;
      s.aruco_x = x;
      s.aruco_y = y;
      s.aruco_theta = theta;

      if (changed) s.newData = true; // only flag when something actually changed
    }
    else if (strcmp(op, "info") == 0){
      const char* loc = doc["mission_loc"] | "bottom";
      const float xf = 0.55f;
      const float yf = (strcmp(loc,"bottom")==0) ? 0.55f : 1.45f;
      const float tf = 0.0f;
      sendArucoFloatsToArduino(xf,yf,tf);
    }
    else if (strcmp(op, "aruco_confirm") == 0){
      s.arucoConfirmed = true;
    }
  }

  void sendArucoFloatsToArduino(float x, float y, float theta){
    union { float f; uint8_t b[4]; } f;
    Serial.write(0x05); Serial.flush();
    f.f = x;     Serial.write(f.b, 4); Serial.flush();
    f.f = y;     Serial.write(f.b, 4); Serial.flush();
    f.f = theta; Serial.write(f.b, 4); Serial.flush();
  }

  State& s;
  WebsocketsClient client;
};
