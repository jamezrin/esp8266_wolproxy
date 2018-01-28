#include <Arduino.h>
#include <WiFiUdp.h>
#include <settings.h>

/*
* Wake on Lan Packet
* FF FF FF FF FF FF    x1    (Frame)        6 bytes
* AA BB CC DD EE FF    x16   (Target MAC)   96 bytes
* FF EE DD CC BB AA    x1    (SecureOn)     6 bytes
* More info https://en.wikipedia.org/wiki/Wake-on-LAN
* Total: 102 bytes without SecureOn, 108 bytes with SecureOn
*/

WiFiUDP con;

void setup() {
  Serial.begin(115200);
  Serial.println();

  if (staticConnection) {
    WiFi.config(
      address,
      gateway,
      netmask
    );
  }

  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA); //Client mode, otherwise it will work as an access point too

  Serial.printf("Connecting to %s\n", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() !=  WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
  }

  Serial.println("\nSuccessfully connected");
  con.begin(port);
  Serial.printf("Listening for packets, listening on %s:%d\n", WiFi.localIP().toString().c_str(), port);
}

bool check(char packet[], int size) {
  //Frame check
  for (int i = 0; i < 6; i++) {
    if (packet[i] != 0xFF)
    return false;
  }

  //Payload check
  for (int i = 6; i < 12; i++) {
    for (int j = 1; j < 16; j++) {
      int pos = i + (6 * j);
      if (packet[i] != packet[pos]) {
        return false;
      }
    }
  }

  //SecureOn check
  if (enforcePassword) {
    if (size >= 108) {
      for (int i = 0; i < 6; i++) {
        if (packet[i + 102] != secureOn[i]) {
          return false;
        }
      }
    } else {
      return false;
    }
  }

  return true;
}

void loop() {
  int packetSize = con.parsePacket();
  if (packetSize) {
    Serial.printf("Received a packet of %d bytes from %s:%d\n",
      packetSize,
      con.remoteIP().toString().c_str(),
      con.remotePort()
    );

    if (packetSize >= 102) {
      char packet[packetSize];
      con.read(packet, packetSize);

      Serial.println("The packet received packet might be valid, going further...");

      if (check(packet, packetSize)) {
        Serial.println("The packet is valid, forwarding to the target...");

        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);

        con.beginPacket(targetAddress, targetPort);
        con.write(packet, packetSize);
        con.endPacket();
      } else {
        Serial.println("The packet received is not valid");
      }
    } else {
      Serial.println("The packet received is too small");
    }
  }
}
