#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "Constants.h"
#include "Net/Wifi.h"
#include "Net/WebSocketClient.h"
#include "Utils/Log.h"

namespace Enes100Lib {

class Enes100 {
  public:
    Enes100(): _x(0), _y(0), _theta(0), _visible(false),
               _wifiOK(false), _team(""), _missionId(-1), _missionStr(""),
               _aruco(-1), _room(-1), _wsStarted(false), _lastAnnounce(0) {}

    bool begin(const String& teamName, const String& missionType, int arucoId, int roomNum){
      _team = teamName;
      int tryId = Enes100Consts::normalizeMission(missionType);
      if (tryId >= 0) { _missionId = tryId; _missionStr = ""; }
      else { _missionStr = missionType; _missionId = -1; }
      _aruco = arucoId; _room = roomNum;

      ELog::info("[Enes100] begin(): connecting Wi‑Fi…");
      _wifiOK = ENet::ensureWifi();
      if (!_wifiOK){ ELog::error("[Enes100] Wi‑Fi failed"); return false; }

      if (!_wsStarted){
        _wsStarted = true;
        _ws.begin(ENES100_WS_URL, [this](const String& msg){ this->handleMessage(msg); });
        _lastAnnounce = 0;
      }
      return true;
    }

    void loop(){
      if (!_wifiOK) return;
      _ws.loop();
      if (_ws.connected()){
        unsigned long now = millis();
        if (_lastAnnounce == 0 || now - _lastAnnounce > 5000){
          announce();
          _lastAnnounce = now;
        }
      }
    }

    int mission(const String& m) { return Enes100Consts::normalizeMission(m); }

    void print(const String& msg){
      if (!_ws.connected()) return;
      StaticJsonDocument<256> d;
      d["op"] = "print";
      d["msg"] = msg;
      d["team"] = _team;
      String out; serializeJson(d, out);
      _ws.send(out);
    }

    bool isConnected() const { return _wifiOK && _ws.connected(); }
    float getX() const { return _x; }
    float getY() const { return _y; }
    float getTheta() const { return _theta; }
    bool isVisible() const { return _visible; }

  private:
    void announce(){
      StaticJsonDocument<256> d;
      d["op"] = "hello";
      d["team"] = _team;
      if (_missionId >= 0) d["mission"] = _missionId;
      else d["mission"] = _missionStr;
      d["aruco"] = _aruco;
      d["room"] = _room;
      String out; serializeJson(d, out);
      _ws.send(out);
    }

    void handleMessage(const String& text){
      StaticJsonDocument<512> d;
      DeserializationError err = deserializeJson(d, text);
      if (err) return;
      const char* op = d["op"] | "";
      if (strcmp(op, "aruco")==0){
        _x = d["x"] | _x;
        _y = d["y"] | _y;
        _theta = d["theta"] | _theta;
        _visible = d["visible"] | _visible;
      } else if (strcmp(op, "pong")==0){
        /* keepalive */ 
      } else {
        if (ENES100_DEBUG) ELog::debug(String("[Enes100] Unknown op: ")+op);
      }
    }

    // state
    volatile float _x,_y,_theta;
    volatile bool _visible;
    bool _wifiOK;
    String _team; int _missionId; String _missionStr;
    int _aruco, _room;
    bool _wsStarted;
    unsigned long _lastAnnounce;

    ENet::WSClient _ws;
};

} // namespace
