import network
import socket
import time

SSID = 'NewHolland'
PASSWORD = 'yoloyolo'

ip_address = "192.168.1.2"

# --- Connect to WiFi ----
# Disable access point mode
ap = network.WLAN(network.AP_IF)
ap.active(False)
# Enable station mode
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, PASSWORD)
wlan.ifconfig((ip_address, "255.255.255.0", "192.168.1.1", "192.168.1.1"))

# --- Init UDP connection ---
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


def blackout():
    sock.sendto(bytes([0, 0, 0, 0, 0]), ("192.168.1.100", 1811))


def flash(h, s, v):
    sock.sendto(bytes([1, 0, h, s, v]), ("192.168.1.100", 1811))


def onehot(k):
    sock.sendto(bytes([2, k, 0, 0, 0]), ("192.168.1.100", 1811))


def sweep(tempo):
    for k in range(16):
        onehot(k * 16)
        time.sleep(60 / tempo)
