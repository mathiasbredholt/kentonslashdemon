import socket
import time
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


def blink():
    start = round(time.time() * 1000)
    sock.sendto(bytes([0, 0, 0, 254, 0, 0, 0, 0]),
                ("192.168.1.100", 1811))

    time.sleep(0.2)
    while 1:
        dt = round(time.time() * 1000) - start + 200
        b1 = (dt & 0xff0000) >> 16
        b2 = (dt & 0x00ff00) >> 8
        b3 = dt & 0x0000ff

        sock.sendto(bytes([b1, b2, b3, 1, 0, 255, 0, 255]),
                    ("192.168.1.100", 1811))
        time.sleep(0.01)
        dt = round(time.time() * 1000) - start + 200
        b1 = (dt & 0xff0000) >> 16
        b2 = (dt & 0x00ff00) >> 8
        b3 = dt & 0x0000ff
        sock.sendto(bytes([b1, b2, b3, 0, 255, 0, 255]),
                    ("192.168.1.100", 1811))
        time.sleep(0.08)
