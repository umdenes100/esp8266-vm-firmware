#pragma once
#include <Arduino.h>
#include <map>

namespace Enes100Consts {
// Populate your mission mapping here if you want name->id translation.
static std::map<String,int> MISSION_BY_NAME = {
    // {"mission_name", 1},
    // {"another", 2},
};

inline int normalizeMission(const String& m) {
  // Try integer
  char* endptr = nullptr;
  long v = strtol(m.c_str(), &endptr, 10);
  if (endptr && *endptr == '\0') return (int)v;
  auto it = MISSION_BY_NAME.find(m);
  if (it != MISSION_BY_NAME.end()) return it->second;
  return -1; // unknown -> -1 (will still be sent as string via missionStr)
}
} // namespace
