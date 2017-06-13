// #include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define NUM_LEDS 16
#define DATA_PIN 3

// CRGB leds[NUM_LEDS];

IPAddress staticIP(192, 168, 1, 2);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress remoteIP(192, 168, 1, 100);

const char* ssid = "NewHolland";
const char* password = "yoloyolo";

char packet1[5] = { 2, 0, 0, 0, 0 };
char packet2[5] = { 1, 0, 0, 0, 0 };

WiFiUDP Udp;
unsigned int localUdpPort = 1811;
char incomingPacket[5];

void setup() {
    // Initialize LEDs
    // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

    // Setup serial
    Serial.begin(115200);
    Serial.println();

    // Setup WiFi
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, password);
    WiFi.config(staticIP, gateway, subnet);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    // Setup UDP
    // Udp.begin(localUdpPort);
}

void loop() {
    while (true) {
        Udp.beginPacket(remoteIP, 1811);
        Udp.write(packet1);
        Udp.endPacket();

        delay(100);

        Udp.beginPacket(remoteIP, 1811);
        Udp.write(packet2);
        Udp.endPacket();

        delay(100);
    }
}