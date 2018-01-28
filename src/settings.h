#include <ESP8266WiFi.h>

const char* hostname = "WakeOnLan";
const char* ssid = "YourWiFiAP";
const char* password = "superpass123";
const int port = 5009;

const bool staticConnection = true;
const IPAddress address = IPAddress(192, 168, 0, 10);
const IPAddress gateway = IPAddress(192, 168, 0, 1);
const IPAddress netmask = IPAddress(255, 255, 255, 0);

const IPAddress targetAddress = IPAddress(255, 255, 255, 255);
const int targetPort = 9;

const char secureOn[6] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6};
const bool enforcePassword = true;
