#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/*
* Wake on Lan Packet
* FF FF FF FF FF FF    x1    (Frame)        6 bytes
* AA BB CC DD EE FF    x16   (Target MAC)   96 bytes
* FF EE DD CC BB AA    x1    (SecureOn)     6 bytes
* More info https://en.wikipedia.org/wiki/Wake-on-LAN
* Total: 102 bytes without secureon, 108 bytes with SecureOn
*/

#define DEVICE_HOSTNAME         "WakeOnLan"
#define WIFI_SSID               "ssidwifi"
#define WIFI_PASSWORD           "superpass123"
#define WOL_PORT                5009
#define STATIC_CONNECTION       false
#define CLIENT_ADDRESS          IPAddress(192, 168, 0, 150)
#define CLIENT_GATEWAY          IPAddress(192, 168, 0, 1)
#define CLIENT_NETMASK          IPAddress(255, 255, 255, 0)
#define TARGET_ADDRESS          IPAddress(255, 255, 255, 255)
#define TARGET_PORT             9

WiFiUDP con;

bool force_password = true;
char password[6] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6}; //Equivalent to A1:B2:C3:D4:E5:F6

bool isValid(char packet[]) {
  //Frame check
  for (int i = 0; i < 6; i++) {
    if (packet[i] != 0xFF)
      return false;
  }

  //Payload check
  for (int i = 6; i < 12; i++) {
    for (int j = 1; j < 16; j++) {
      int pos = i + (6 * j);
      if (packet[i] != packet[pos])
        return false;
    }
  }

  //SecureOn check
  if (force_password) {
    if (sizeof(packet) == 108) {
      for (int i = 0; i < 6; i++) {
        if (packet[i + 102] != password[i]) {
          return false;
        }
      }
    } else {
      return false;
    }
  }

  return true;
}

bool checkConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Lost connection to the network, resetting...");
    ESP.reset();
    return false;
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  if (STATIC_CONNECTION) {
    WiFi.config(
      CLIENT_ADDRESS,
      CLIENT_GATEWAY,
      CLIENT_NETMASK
    );
  }

  WiFi.hostname(DEVICE_HOSTNAME);
  WiFi.mode(WIFI_STA); //Client mode, otherwise it will work as an access point too

  Serial.printf("Connecting to %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() !=  WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
  }

  Serial.println("\nConectado correctamente");
  con.begin(WOL_PORT);
  Serial.printf("Esperando paquetes, mi IP %s, y puerto %d\n", WiFi.localIP().toString().c_str(), WOL_PORT);
}

void loop() {
  if (checkConnection()) {
    int size = con.parsePacket();
    if (size) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.printf("Received packet from %s:%d\n", con.remoteIP().toString().c_str(), con.remotePort());
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);

      if (size == 102 || size == 108) {
        char buffer[size];
        con.read(buffer, size);

        Serial.println("The packet received packet might be valid, going further...");

        if (isValid(buffer)) {
          Serial.println("The packet is valid, forwarding to the target...");

          digitalWrite(LED_BUILTIN, HIGH);
          delay(250);
          digitalWrite(LED_BUILTIN, LOW);

          con.beginPacket(TARGET_ADDRESS, TARGET_PORT);
          con.write(buffer, size);
          con.endPacket();
        } else {
          Serial.println("The packet received is not valid");
        }
      }
    }
  }
}
