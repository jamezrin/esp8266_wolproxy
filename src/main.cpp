#include <Arduino.h>
#include <WiFiUdp.h>
#include <settings.h>
#include <EasyDDNS.h>

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
  Serial.setDebugOutput(true);

  if (Settings::staticConnection) {
    WiFi.config(
      Settings::address,
      Settings::gateway,
      Settings::netmask
    );
  }

  WiFi.hostname(Settings::hostname);
  WiFi.mode(WIFI_STA); //Client mode, otherwise it will work as an access point too

  Serial.printf("Connecting to %s\n", Settings::ssid);
  WiFi.begin(
    Settings::ssid,
    Settings::password
  );

  while (WiFi.status() !=  WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
  }

  Serial.println("\nSuccessfully connected");

  EasyDDNS.service("noip");
  EasyDDNS.client(
    Settings::ddnsHostname,
    Settings::ddnsUsername,
    Settings::ddnsPassword
  );

  con.begin(Settings::port);
  Serial.printf("Listening for packets on %s:%d\n",
    WiFi.localIP().toString().c_str(),
    Settings::port
  );
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
  if (Settings::enforcePassword) {
    if (size >= 108) {
      for (int i = 0; i < 6; i++) {
        if (packet[i + 102] != Settings::secureOn[i]) {
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
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Device is not connected to wifi and does not reconnect");
  }

  EasyDDNS.update(Settings::ddnsInterval);

  int packetSize = con.parsePacket();
  if (packetSize) {
    Serial.printf("Received a packet of %d bytes from %s:%d\n",
      packetSize,
      con.remoteIP().toString().c_str(),
      con.remotePort()
    );

    if (packetSize >= 102) {
      char packet[packetSize];
      int len = con.read(packet, 255);
      if (len > 0) {
        packet[len] = 0;
      }

      Serial.println("The packet received packet might be valid, going further...");

      if (check(packet, packetSize)) {
        Serial.println("The packet is valid, forwarding to the target...");

        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);

        con.beginPacket(
          Settings::targetAddress,
          Settings::targetPort
        );

        con.write(packet, packetSize);
        con.endPacket();
      } else {
        Serial.println("The packet received is not valid");
      }
    } else {
      Serial.println("The packet received is too small");
    }
  } else {
    Serial.print(".");
    delay(1000);
  }
}
