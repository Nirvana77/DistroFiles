import PySimpleGUI as sg
import sys
import socket
from time import sleep
import client as c
import payload as p
from struct import *

layout = [
	[
		sg.Canvas(key="main", size=(1000,500))
	],
	[
		sg.Button("OK"),
		sg.Button("Send File", key="send"),
		sg.Button("Get list", key="list")
	]
]

# Create the window
window = sg.Window("GUI", layout)
s = c.connect("mc.gamingpassestime.com", 7000)

# Create an event loop
while True:
	try:
		msg = s.recv(4096)
	except socket.timeout as e:
		err = e.args[0]
		# this next if/else is a bit redundant, but illustrates how the
		# timeout exception is setup
		if err == 'timed out':
			sleep(1)
			print('recv timed out, retry later')
		else:
			print(e)
	except socket.error as e:
		# Something else happened, handle error, exit, etc.
		print(e)
	else:
		if len(msg) == 0:
			print('orderly shutdown on server end')
		else:
			(uuid, src, des, method, message) = p.recive(msg)
			print("UUID: ", uuid)
			print("src: ", src)
			print("des: ", des)
			print("method: ", method)
			print("message: ", message)
	event, values = window.read()
	# End program if user closes window or
	# presses the OK button
	if event == "OK" or event == sg.WIN_CLOSED:
		break
	elif event == "send":
		p.send_file(s, "C:\\Users\\Navanda77\\school\\Filesystem\\Tests\\test.json", "test.json")
	elif event == "list":
		p.get_list(s, "root")
	else:
		print("Event: ", event)
		print("Values: ", values)
		window["date"].update("mamasd")

window.close()
s.close()