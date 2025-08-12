#pragma once
#include <Arduino.h>

namespace ELog {
  inline void debug(const String& s){ if (ENES100_DEBUG) Serial.println("[DBG] " + s); }
  template<typename... Args> inline void debugf(const char* fmt, Args... args){
    if (!ENES100_DEBUG) return;
    char buf[256]; snprintf(buf,sizeof(buf),fmt,args...); Serial.print("[DBG] "); Serial.println(buf);
  }
  inline void info(const String& s){ Serial.println("[LOG] " + s); }
  inline void warn(const String& s){ Serial.println("[WRN] " + s); }
  inline void error(const String& s){ Serial.println("[ERR] " + s); }
}
