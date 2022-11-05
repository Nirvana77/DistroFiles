import PySimpleGUI as sg
import sys
import socket
from time import sleep
import client as c
import payload as p
from struct import *
import memory as m

# Canvas.create_polygon
# Canvas.create_oval
# Canvas.create_line
# Canvas.create_image
# Canvas.create_bitmap
# Canvas.create_arc
# Canvas.create_rectangle
# Canvas.create_text
# Canvas.create_window

def draw_Directory(canvas, dir):
	(x, y, w, h, margin) = (0, 0, 100, 100, 10) 
	file_color = "RED"
	folder_color = "GREEN"
	for d in dir:
		rec = canvas.create_rectangle(x, y, w + x, h + y, fill=file_color)
		
		if not d["isFile"]:
			canvas.itemconfig(rec, fill=folder_color)
		canvas.create_text(x + w / 2, y + h + 15, text=d["name"])
		x += w + margin
		
	print(dir)
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
	if len(data) != 0:
		
		if method == "listRespons":
			(i, size) = m.get_UInt16(data, 0)
			directory = []
			for x in range(0, size):
				(i, isFile) = m.get_UInt8(data, i)
				(i, size) = m.get_UInt16(data, i)
				(i, name) = m.get_Seting(data, i, size)
				obj = {
					"isFile": isFile == 1,
					"name": name,
				}
				directory.append(obj)
			draw_Directory(window["main"].TKCanvas, directory)

		print("Data")
	
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