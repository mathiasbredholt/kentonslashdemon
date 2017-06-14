s#include <Scheduler.h>

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <QueueArray.h>


#define NUM_LEDS 16
#define DATA_PIN 3
#define UDP_PORT 1811
#define DGRAM_LENGTH 8
#define MAX_LED_EVENTS 4
#define TIME_RESOLUTION 5

CRGBArray<NUM_LEDS> leds;

IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ssid = "NewHolland";
const char* password = "yoloyolo";

WiFiUDP Udp;
uint8_t dgram[8];

int now;

struct LEDEvent {
    int timestamp;
    uint8_t program;
    uint8_t phase;
    CHSV hsv;
};

WiFiClient

// struct LEDEvent led_events[MAX_LED_EVENTS];
// uint8_t evt_idx;

QueueArray <LEDEvent> led_events;


void onehot(int phase, CHSV hsv) {
    int i = 0;
    leds(i, i).fill_solid(hsv);
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i == phase) {
            leds(i, i).fill_solid(hsv);
        } else if (i == phase - 1 || i == phase + 1) {
            leds(i, i) = leds(phase, phase);
            // leds[i] %= 128; // Dim to 50 % 128/255
        } else if (i == phase - 2 || i == phase + 2) {
            leds(i, i) = leds(phase, phase);
            // leds[i] %= 64; // Dim to 25 % 64/255
        } else {
            leds(i, i).fill_solid(CRGB::Black);
        }
    }
}

// void sweep(int k, CHSV hsv, int tempo) {

// }

void noise(int k, int hue_scale, int time) {
    uint16_t = time = uint16_t(time);
    int octaves = 3;







    fill_noise8 (leds, NUM_LEDS, uint8_t octaves, uint16_t x, int scale, uint8_t hue_octaves, uint16_t hue_x, int hue_scale, uint16_t time)
}

class TimerTask : public Task {
protected:
    void setup() {
        now = 0;
    }

    void loop() {
        now += TIME_RESOLUTION;
        delay(TIME_RESOLUTION);
    }
} timer_task;

class SchedTask : public Task {
protected:
    void setup() {
    }

    void loop() {
        if (!led_events.isEmpty()) {
            struct LEDEvent evt = led_events.front();
            if (evt.timestamp - now < TIME_RESOLUTION || evt.timestamp == 0) {
                // printf("%d, %d, %d, %d\n", now, evt.timestamp, evt.program, evt.phase);

                switch (evt.program) {
                case 0: // Blackout
                    leds.fill_solid(CRGB::Black);
                    break;
                case 1: // Flash
                    leds.fill_solid(evt.hsv);
                    break;
                case 2: // Onehot
                    onehot(evt.phase, evt.hsv);
                    break;
                case 254:
                    now = 0;
                    break;
                }
                // printf("%d, %d, %d, %d\n", now, evt.timestamp, evt.program, evt.phase);
                FastLED.show();
                led_events.pop();
            }
        }

    }
} sched_task;

class UDPTask : public Task {
protected:
    void setup() {
        // Initialize UDP
        Udp.begin(UDP_PORT);
    }

    void loop() {
        int packet_size = Udp.parsePacket();
        if (packet_size) {
            int len = Udp.read(dgram, DGRAM_LENGTH);
            if (len > 0) {
                int timestamp = (dgram[0] << 16) | (dgram[1] << 8) | dgram[2];
                uint8_t program = dgram[3];
                uint8_t phase = dgram[4];
                CHSV hsv = CHSV(dgram[5], dgram[6], dgram[7]);
                // printf("Received program %d\n", program);

                struct LEDEvent evt;
                evt.timestamp = timestamp;
                evt.program = program;
                evt.phase = phase;
                evt.hsv = hsv;

                led_events.push(evt);
            }
        }
    }
} udp_task;

void setup() {
    Serial.begin(115200);
    Serial.println();

    // ---- Network configuration ----
    // Read binary settings file
    // c0 a8 01 64 : 192 168 1 100
    SPIFFS.begin();
    File f = SPIFFS.open("/settings", "r");
    char data[4];
    f.readBytes(data, 4);
    SPIFFS.end();

    IPAddress staticIP(data[0], data[1], data[2], data[3]);

    // Connect to WiFi
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, password);
    WiFi.config(staticIP, gateway, subnet);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    // Initialize LEDs
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);


    // Initialize scheduling
    Scheduler.start(&timer_task);
    Scheduler.start(&sched_task);
    Scheduler.start(&udp_task);
    Scheduler.begin();
}

void loop() {}