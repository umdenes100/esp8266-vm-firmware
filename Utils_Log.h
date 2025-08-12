#pragma once
#include "Config.h"

namespace ELog {

  inline void info(const String& s){
  #ifdef DEBUG
    Serial.println("[LOG] " + s);
  #endif
  }

  inline void warn(const String& s){
  #ifdef DEBUG
    Serial.println("[WRN] " + s);
  #endif
  }

  inline void error(const String& s){
  #ifdef DEBUG
    Serial.println("[ERR] " + s);
  #endif
  }

  template<typename... Args>
  inline void debugf(const char* fmt, Args... args){
  #ifdef DEBUG
    char buf[256];
    snprintf(buf, sizeof(buf), fmt, args...);
    Serial.print("[DBG] ");
    Serial.println(buf);
  #endif
  }

  inline void debug(const String& s){
  #ifdef DEBUG
    Serial.println("[DBG] " + s);
  #endif
  }

} // namespace ELog
