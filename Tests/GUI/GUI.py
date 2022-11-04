import PySimpleGUI as sg
import sys
import socket
from time import sleep
import client as c
import payload as p
from struct import *
import memory as m

layout = [
	[
		sg.Canvas(key="main", size=(1000,500))
	],
	[
		sg.Button("Exit"),
		sg.Button("Send File", key="send"),
		sg.Button("Get list", key="list")
	]
]

# Create the window
window = sg.Window("GUI", layout)
s = c.connect("mc.gamingpassestime.com", 7000)

# Create an event loop
while True:
	(method, data) = c.work(s)
	
	event, values = window.read()
	# End program if user closes window or
	# presses the OK button
	if event == "Exit" or event == sg.WIN_CLOSED:
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