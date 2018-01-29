#include <ESP8266WiFi.h>

namespace Settings {
  static const char* hostname = "WakeOnLan";
  static const char* ssid = "YourWiFiAP";
  static const char* password = "superpass123";
  static const int port = 5009;

  static const bool staticConnection = true;
  static const IPAddress address = IPAddress(192, 168, 0, 10);
  static const IPAddress gateway = IPAddress(192, 168, 0, 1);
  static const IPAddress netmask = IPAddress(255, 255, 255, 0);

  static const IPAddress targetAddress = IPAddress(255, 255, 255, 255);
  static const int targetPort = 9;

  static const char secureOn[6] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6};
  static const bool enforcePassword = true;
};
