// QuickStart example for Enes100-ESP8266
// Install dependencies: ArduinoJson, WebSockets by Markus Sattler
// In your sketch, you can override defaults by defining ENES100_* before including Enes100.h
// e.g.:
//   #define ENES100_WIFI_SSID "ssid"
//   #define ENES100_WIFI_PASS "pass"
//   #define ENES100_WS_URL "ws://192.168.4.1:8765/ws"
//   #define ENES100_DEBUG 1

#include <Enes100/Enes100.h>

Enes100Lib::Enes100 enes;

void setup(){
  Serial.begin(115200);
  delay(500);
  Serial.println("\n== Enes100 QuickStart ==");

  // Provide your own values:
  String team = "TeamName";
  String mission = "1"; // or "mission_name"
  int aruco = 0;
  int room = 0;

  enes.begin(team, mission, aruco, room);
}

void loop(){
  enes.loop();

  static unsigned long last = 0;
  if (millis() - last > 2000){
    last = millis();
    if (enes.isConnected()){
      enes.print("Hello from ESP8266!");
      Serial.print("Pose: "); Serial.print(enes.getX()); Serial.print(", "); Serial.print(enes.getY());
      Serial.print("  theta="); Serial.print(enes.getTheta());
      Serial.print("  visible="); Serial.println(enes.isVisible() ? "yes":"no");
    } else {
      Serial.println("Connecting…");
    }
  }
}
