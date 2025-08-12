#pragma once
#include <Arduino.h>
#include <WebSocketsClient.h>
#include "../Utils/Log.h"

namespace ENet {
  class WSClient {
   public:
    using MsgHandler = std::function<void(const String&)>;
    WSClient(): _connected(false), _lastPong(0) {}

    void begin(const String& url, MsgHandler onMessage){
      _onMessage = onMessage;
      parseUrl(url); // fills _host/_path/_port/_ssl
      _ws.onEvent([this](WStype_t type, uint8_t * payload, size_t length){
        switch(type){
          case WStype_CONNECTED:
            _connected = true; 
            _lastPong = millis();
            ELog::info("[WS] Connected");
            break;
          case WStype_DISCONNECTED:
            _connected = false;
            ELog::warn("[WS] Disconnected");
            break;
          case WStype_TEXT:
            if (_onMessage) _onMessage(String((const char*)payload, length));
            break;
          case WStype_PONG:
            _lastPong = millis();
            break;
          default: break;
        }
      });
      if (_ssl) _ws.beginSSL(_host.c_str(), _port, _path.c_str());
      else _ws.begin(_host.c_str(), _port, _path.c_str());
      _ws.setReconnectInterval(500); // managed in loop for backoff
      _ws.enableHeartbeat(15000, 3000, 2); // ping every 15s
    }

    void loop(){ _ws.loop(); }
    bool connected() const { return _connected; }
    bool send(const String& s){ return _ws.sendTXT(s); }
    unsigned long lastPongMs() const { return _lastPong; }

   private:
    void parseUrl(const String& url){
      // crude url parse: ws://host:port/path or wss://host/path
      _ssl = url.startsWith("wss://");
      String rest = url.substring(_ssl ? 6 : 5);
      int slash = rest.indexOf('/');
      String hostport = (slash>=0 ? rest.substring(0,slash) : rest);
      _path = (slash>=0 ? rest.substring(slash) : "/");
      int colon = hostport.indexOf(':');
      _host = (colon>=0 ? hostport.substring(0,colon) : hostport);
      _port = (colon>=0 ? hostport.substring(colon+1).toInt() : (_ssl?443:80));
    }

    WebSocketsClient _ws;
    bool _ssl;
    bool _connected;
    unsigned long _lastPong;
    String _host, _path; uint16_t _port;
    MsgHandler _onMessage;
  };
}
