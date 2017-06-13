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


def blink(tempo):
    flash(255, 255, 255)
    time.sleep(10 / tempo)
    blackout()
    time.sleep(90 / tempo)
