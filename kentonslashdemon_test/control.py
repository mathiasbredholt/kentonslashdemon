import socket
import time
import threading

# ------ Programs -------
Blackout = 0
Flash    = 1
OneHot   = 2
Noise    = 3



sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


def blink(client):
	start = round(time.time() * 1000)
	client.send(ledcmd(254))

	time.sleep(0.2)
	while 1:
		dt = round(time.time() * 1000) - start + 50
		client.send(ledcmd(0, timestamp=dt))
		time.sleep(0.04)
		dt = round(time.time() * 1000) - start + 50
		client.send(ledcmd(1, timestamp=dt))
		time.sleep(0.005)


def start():
	sock.bind(("192.168.1.51", 1811))
	sock.listen(1)

	(client, address) = sock.accept()
	return client


def blink2(client):
	client.send(ledcmd(0))
	time.sleep(0.25)
	client.send(ledcmd(1))
	time.sleep(0.25)


def sweep(client):
	start = round(time.time() * 1000)
	client.send(ledcmd(254))

	time.sleep(0.2)
	while 1:
		for i in  range(0,15):
			dt = round(time.time() * 1000) - start + 50
			client.send(ledcmd(OneHot, i, dt,[0,0,100]))
			time.sleep(0.1)



def ledcmd(program=0, phase=0, timestamp=0, hsv=[0, 0, 20]):
	b1 = (timestamp & 0xff0000) >> 16
	b2 = (timestamp & 0x00ff00) >> 8
	b3 = timestamp & 0x0000ff
	return bytes([b1, b2, b3, program, phase, hsv[0], hsv[1], hsv[2]])

# sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# def blackout():
#     sock.sendto(bytes([0, 0, 0, 0, 0]), ("192.168.1.100", 1811))

# def flash(h, s, v):
#     sock.sendto(bytes([1, 0, h, s, v]), ("192.168.1.100", 1811))

# def onehot(k):
#     sock.sendto(bytes([2, k, 0, 0, 0]), ("192.168.1.100", 1811))

# def sweep(tempo):
#     for k in range(16):
#         onehot(k * 16)
#         time.sleep(60 / tempo)

# def blink():
#     start = round(time.time() * 1000)
#     sock.sendto(bytes([0, 0, 0, 254, 0, 0, 0, 0]),
#                 ("192.168.1.100", 1811))

#     time.sleep(0.2)
#     while 1:
#         dt = round(time.time() * 1000) - start + 200
#         b1 = (dt & 0xff0000) >> 16
#         b2 = (dt & 0x00ff00) >> 8
#         b3 = dt & 0x0000ff

#         sock.sendto(bytes([b1, b2, b3, 1, 0, 255, 0, 255]),
#                     ("192.168.1.100", 1811))
#         time.sleep(0.01)
#         dt = round(time.time() * 1000) - start + 200
#         b1 = (dt & 0xff0000) >> 16
#         b2 = (dt & 0x00ff00) >> 8
#         b3 = dt & 0x0000ff
#         sock.sendto(bytes([b1, b2, b3, 0, 255, 0, 255]),
#                     ("192.168.1.100", 1811))
#         time.sleep(0.08)
