#include <Scheduler.h>

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <QueueArray.h>


#define NUM_LEDS 16
#define DATA_PIN 3 // D0
#define UDP_PORT 1811
#define TCP_PORT 1811
#define DGRAM_LENGTH 8
#define MAX_LED_EVENTS 4
#define TIME_RESOLUTION 5
#define PACKET_LENGTH 8

CRGBArray<NUM_LEDS> leds;

IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress server(192, 168, 1, 51);

// const char* ssid = "D J  $ K Y W 4 L K 3 R";
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

// Variables for noise
uint16_t x;
uint16_t y;
uint16_t z;


CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette(CloudColors_p);
TBlendType    currentBlending = LINEARBLEND;



WiFiClient client;

// struct LEDEvent led_events[MAX_LED_EVENTS];
// uint8_t evt_idx;

QueueArray <LEDEvent> led_events;


void onehot(int phase, CHSV hsv) {
    int i = 0;
    leds.fill_solid(CRGB::Black); // Turn off all led's
    leds(i, i).fill_solid(hsv);
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i == phase) {
            leds(i, i).fill_solid(hsv);
        } else if (i == phase - 1 || i == phase + 1) {
            leds(i, i) = leds(phase, phase);
            leds[i] %= 128; // Dim to 50 % 128/255
        } else if (i == phase - 2 || i == phase + 2) {
            leds(i, i) = leds(phase, phase);
            leds[i] %= 64; // Dim to 25 % 64/255
        } else {
            leds(i, i).fill_solid(CRGB::Black);
        }
    }
}

void fade(int phase, int tempo, uint8_t h, uint8_t s, uint8_t v) {
    float delta = v - leds.[phase].v;
    int inc = delta / tempo
    for (int i = 0, i < tempo, i++) {
        leds[phase].v +=
    }
}


void ChangePalettePeriodically()
{

    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if ( lastSecond != secondHand) {
        lastSecond = secondHand;
        //if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if ( secondHand == 10)  { targetPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if ( secondHand == 20)  { targetPalette = ForestColors_p;           currentBlending = LINEARBLEND; }
        if ( secondHand == 30)  { targetPalette = LavaColors_p;           currentBlending = LINEARBLEND; }
        if ( secondHand == 40)  { targetPalette = OceanColors_p;           currentBlending = LINEARBLEND; }
        if ( secondHand == 50)  { targetPalette = HeatColors_p;           currentBlending = LINEARBLEND; }
    }
}

void fillnoise8(int hue_scale, int speed, int bri_scale ) {

    uint8_t dataSmoothing = 0;
    if ( speed < 50) {
        dataSmoothing = 200 - (speed * 4);
    }

    ChangePalettePeriodically();
    nblendPaletteTowardPalette(currentPalette, targetPalette, 10);

    for (int i = 0; i < NUM_LEDS; i++) {
        int dhue = hue_scale * i;
        int dbri = bri_scale * i;

        uint8_t hue = inoise8(x + dhue, z);
        uint8_t bri = inoise8(y + dbri, z);
        // Rescale
        hue = qsub8(hue, 16);
        hue = qadd8(hue, scale8(hue, 39));
        bri = qsub8(hue, 16);
        bri = qadd8(hue, scale8(hue, 39));

        int hueSmoothing = 1;
        if ( hueSmoothing ) {
            uint8_t oldhue = leds[i];
            uint8_t newhue = scale8( oldhue, hueSmoothing) + scale8( hue, 256 - hueSmoothing);
            hue = newhue;
        }

        leds[i] = ColorFromPalette(currentPalette, hue, bri, currentBlending);
    }

    z += speed;

    // apply slow drift to X and Y, just for visual variation.
    x += speed / 8;
    y += speed / 16;

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
                case 3: // Noise
                    // hue_scale = evt.hsv.h;
                    // int speed = evt.hsv.s;
                    // int bri_scale = evt.hsv.v;
                    fillnoise8(evt.hsv.h, evt.hsv.s, evt.hsv.v);
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
        if (client.available() >= PACKET_LENGTH) {
            for (int i = 0; i < PACKET_LENGTH; ++i) {
                dgram[i] = client.read();
            }

            int timestamp = (dgram[0] << 16) | (dgram[1] << 8) | dgram[2];
            uint8_t program = dgram[3];
            uint8_t phase = dgram[4];
            CHSV hsv = CHSV(dgram[5], dgram[6], dgram[7]);
            printf("Received program %d\n", program);

            struct LEDEvent evt;
            evt.timestamp = timestamp;
            evt.program = program;
            evt.phase = phase;
            evt.hsv = hsv;

            led_events.push(evt);

            // printf("c: %d, s: %d\n", led_events.count(), client.available());
        }

        if (!client.connected()) {
            Serial.println("Disconnected.");
            setup();
        }
    }
} tcp_task;


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
    // Scheduler.start(&udp_task);
    Scheduler.start(&tcp_task);
    Scheduler.begin();

    // Noise parameters

    x = random16();
    y = random16();
    z = random16();

}


void loop() {}