#include <Scheduler.h>
#include <FastLED.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <queue>

#define NUM_LEDS 16
#define DATA_PIN 3
#define TCP_PORT 1811
#define UDP_PORT 1812
#define MAX_LED_EVENTS 4
#define TIME_RESOLUTION 5
#define PACKET_LENGTH 8

CRGBArray<NUM_LEDS> leds;

const char* ssid = "NewHolland";
const char* password = "yoloyolo";

Ticker clk1, clk2;
CHSV strobe_hsv;
bool strobe_state;
bool strobe_enabled = false;
int strobe_period = 100;

IPAddress server(192, 168, 1, 50);

struct LEDEvent {
    int timestamp;
    uint8_t program;
    uint8_t phase;
    CHSV hsv;

    bool operator<(const LEDEvent& rhs) const {
        return timestamp > rhs.timestamp;
    }
};

std::priority_queue<LEDEvent> led_events;



WiFiClient client;
WiFiUDP Udp;
uint8_t dgram[8];

int now;


void onehot(int phase, CHSV hsv) {
    int i = 0;
    leds.fill_solid(CRGB::Black); // Turn off all led's
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

void noise(int phase, int hue_scale, int hue_range, int pos_scale) {
    uint16_t time = uint16_t(phase);
    int octaves = 3;
    fill_noise8 (leds, NUM_LEDS, octaves, phase, pos_scale, hue_range, phase, hue_scale, phase);
}

void strobe_on(CHSV hsv) {
    if (!strobe_enabled) {
        strobe_enabled = true;
        strobe_state = false;
        strobe_hsv = hsv;
        clk1.attach_ms(strobe_period, strobe_loop);
    }
}

void strobe_loop() {
    strobe_state = !strobe_state;
    if (strobe_state) {
        leds.fill_solid(strobe_hsv);
        clk2.attach_ms((int) strobe_period * 0.1, strobe_loop);
    } else {
        leds.fill_solid(CRGB::Black);
        clk2.detach();
    }
    FastLED.show();
}

void strobe_off() {
    if (strobe_enabled) {
        strobe_enabled = false;
        leds.fill_solid(CRGB::Black);
        FastLED.show();
        clk1.detach();
        clk2.detach();
    }
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
        if (!led_events.empty()) {
            struct LEDEvent evt = led_events.top();
            if (evt.timestamp - now < TIME_RESOLUTION || evt.timestamp == 0) {
                if (evt.timestamp - now < 0) printf("Fail: %d\n", abs(evt.timestamp - now));
                // printf("%d, %d, %d, %d\n", now, evt.timestamp, evt.program, evt.phase);

                switch (evt.program) {
                case 0: // Solid
                    leds.fill_solid(CHSV(evt.hsv.h, evt.hsv.s, (char) evt.hsv.v * (evt.phase / 255.0)));
                    break;
                case 1: // Onehot
                    onehot(evt.phase, evt.hsv);
                    break;
                case 2: // Noise
                    //noise(evt.phase, hue_scale, hue_range, pos_scale);
                    break;
                case 3: // Fade
                    break;
                case 4: // Strobe on
                    strobe_on(evt.hsv);
                    break;
                case 5: // Strobe off
                    strobe_off();
                    break;
                case 254: // Sync
                    now = 0;
                    break;
                case 255: // Test
                    leds.fill_solid(CRGB::White);
                    break;
                }
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
            int len = Udp.read(dgram, PACKET_LENGTH);
            if (len > 0) {
                int timestamp = (dgram[0] << 16) | (dgram[1] << 8) | dgram[2];
                uint8_t program = dgram[3];
                uint8_t phase = dgram[4];
                CHSV hsv = CHSV(dgram[5], dgram[6], dgram[7]);

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

class TCPTask : public Task {
protected:
    void setup() {
        // Connect to server
        Serial.print("Connecting to server ");
        Serial.print(server);
        while (!client.connect(server, TCP_PORT)) {
            Serial.print(".");
            delay(500);
        }
        Serial.println();
        Serial.println("Connected.");
    }

    void loop() {
        // if (client.available() >= PACKET_LENGTH) {
        //     for (int i = 0; i < PACKET_LENGTH; ++i) {
        //         dgram[i] = client.read();
        //     }

        //     int timestamp = (dgram[0] << 16) | (dgram[1] << 8) | dgram[2];
        //     uint8_t program = dgram[3];
        //     uint8_t phase = dgram[4];
        //     CHSV hsv = CHSV(dgram[5], dgram[6], dgram[7]);
        //     // printf("Received program %d\n", program);

        //     struct LEDEvent evt;
        //     evt.timestamp = timestamp;
        //     evt.program = program;
        //     evt.phase = phase;
        //     evt.hsv = hsv;

        //     led_events.push(evt);

        //     // printf("bytes: %d, now: %d\n", client.available(), now);
        // }

        if (!client.connected()) {
            Serial.println("Disconnected.");
            leds.fill_solid(CRGB::Black);
            FastLED.show();
            setup();
        }

        delay(1000);
    }
} tcp_task;

void setup() {
    Serial.begin(115200);
    Serial.println();

    // Connect to WiFi
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, password);
    // WiFi.config(staticIP, gateway, subnet);
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
    Scheduler.start(&tcp_task);
    Scheduler.begin();
}

void loop() {}