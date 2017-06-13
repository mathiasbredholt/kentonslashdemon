import machine
import neopixel
import network
import socket
import uos
import time

N = 450

SSID = 'NewHolland'
PASSWORD = 'yoloyolo'

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)

# Read static IP from settings file
f = open("settings", "r")
ip_address = f.read()
f.close()

# --- Connect to WiFi ----
# Disable access point mode
ap = network.WLAN(network.AP_IF)
ap.active(False)
# Enable station mode
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, PASSWORD)
wlan.ifconfig((ip_address, "255.255.255.0", "192.168.1.1", "192.168.1.1"))

print(wlan.ifconfig())

# --- Init UDP connection ---
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((ip_address, 1811))

# --- Init NeoPixel ---
np = neopixel.NeoPixel(machine.Pin(0), N)  # D3

# Init timers
timer1 = machine.Timer(-1)
# randomly generate time interval. 500 - 4596

print(uos.urandom(1))

# period = int(machine.rng()) << 12 + 500

# timer1.init(period=period, mode=Timer.ONE_SHOT, callback=strikeon)

timer2 = machine.Timer(-1)

# Init GPIOS
D1 = machine.Pin(5, machine.Pin.OUT)
D2 = machine.Pin(4, machine.Pin.OUT)


# Callbacks
def strikeon():
    # i = machine.rng()
    # if i > 8388608:  # 2^24/2
    #     D1.on()
    # else:
    #     D2.on()
    # timer2.init(period=100, mode=Timer.ONE_SHOT, callback=strikeoff)
    pass


def strikeoff():
    # D1.off()
    # D2.off()
    # # period = int(machine.rng()) << 12 + 500
    # period = uos.urandom(1)

    # timer1.init(period=period, mode=Timer.ONE_SHOT, callback=strikeon)
    pass


def hsv_to_rgb(h, s, v):
    h = h / 255
    s = s / 255
    v = v / 255
    if s == 0.0:
        v = int(v * 255)
        return v, v, v
    i = int(h * 6.0)
    f = (h * 6.0) - i
    p = v * (1.0 - s)
    q = v * (1.0 - s * f)
    t = v * (1.0 - s * (1.0 - f))
    i = i % 6

    p = int(p * 255)
    q = int(q * 255)
    t = int(t * 255)
    v = int(v * 255)

    if i == 0:
        return v, t, p
    if i == 1:
        return q, v, p
    if i == 2:
        return p, v, t
    if i == 3:
        return p, q, v
    if i == 4:
        return t, p, v
    if i == 5:
        return v, p, q


# --- Programs ---
# Black out
def program0(k, h, s, v):
    for i in range(N):
        np[i] = BLACK


# Flash
def program1(k, h, s, v):
    rgb = hsv_to_rgb(h, s, v)
    for i in range(N):
        np[i] = rgb


# One hot
def program2(k, h, s, v):
    rgb = hsv_to_rgb(h, s, v)
    rgb2 = hsv_to_rgb(h, s, int(v / 32))
    rgb3 = hsv_to_rgb(h, s, int(v / 64))
    idx = k >> 4
    for i in range(N):
        if i == idx:
            np[i] = rgb
        elif i == idx - 1 or i == idx + 1:
            np[i] = rgb2
        elif i == idx - 2 or i == idx + 2:
            np[i] = rgb3
        else:
            np[i] = BLACK


# Fade up
def program3(k, h, s, v):
    rgb = hsv_to_rgb(h, s, int(v * k / 255))
    rgb = BLACK
    for i in range(N):
        np[i] = rgb


def program4(k, h, s, v):
    pass


def program5(k, h, s, v):
    pass


def sweep(tempo, h, s, v):
    for k in range(16):
        program2(k * 16, h, s, v)
        np.write()
        time.sleep(60 / tempo)


def fadeup(tempo, h, s, v):
    for k in range(255):
        ticks = time.ticks_ms()
        program3(k, h, s, v)
        print(time.ticks_ms() - ticks)
        # time.sleep(60 / tempo)


# # --- Main loop ---
# while 1:
#     # dgram = sock.recv(5)
#     # p = int(dgram[0])  # program
#     # k = int(dgram[1])  # progress
#     # h = int(dgram[2])  # hue
#     # s = int(dgram[3])  # saturation
#     # v = int(dgram[4])  # intensity

#     # if p == 0:
#     #     program0(k, h, s, v)
#     # elif p == 1:
#     #     program1(k, h, s, v)
#     # elif p == 2:
#     #     program2(k, h, s, v)
#     # elif p == 3:
#     #     program3(k, h, s, v)
#     # elif p == 4:
#     #     program4(k, h, s, v)
#     # elif p == 5:
#     #     program5(k, h, s, v)

#     # np.write()
#     sweep(500)
