#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>

#define NUM_LEDS 16
#define DATA_PIN 3

CRGB leds[NUM_LEDS];

IPAddress staticIP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ssid = "NewHolland";
const char* password = "yoloyolo";

WiFiUDP Udp;
unsigned int localUdpPort = 1811;
char incomingPacket[5];

int now = 0;

Ticker timer0;

void setup() {
    // Initialize LEDs
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

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

    timer0.attach(0.001, tick);

    // Setup UDP
    Udp.begin(localUdpPort);
}

void loop() {
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        int len = Udp.read(incomingPacket, 5);
        if (len > 0) {
            char p = incomingPacket[0];
            char k = incomingPacket[1];
            char h = incomingPacket[2];
            char s = incomingPacket[3];
            char v = incomingPacket[4];

            printf("%d", p);

            if (p == 1) {

            } else if (p == 2) { // Blackout
                for (int i = 0; i < NUM_LEDS; ++i) {
                    leds[i] = CRGB::Black;
                }
            } else if (p == 3) { // Constant color
                for (int i = 0; i < NUM_LEDS; ++i) {
                    leds[i] = CRGB::White;
                }
            }

            FastLED.show();
        }
    }
}

void tick() {
    ++now;
}