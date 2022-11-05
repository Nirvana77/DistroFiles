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
class GUI:

	map = []
	
	def __init__(self, canvas):
		self.canvas = canvas

	def draw_Directory(self, dir):
		(x, y, w, h, margin) = (0, 0, 100, 100, 10) 
		for d in dir:
			
			if d["isFile"]:
				fileFolder = self.File(self.canvas, x, y, w, h, d["name"])
			else:
				fileFolder = self.Folder(self.canvas, x, y, w, h, d["name"])
			

			self.map.append(fileFolder)
			x += w + margin
			
		print(dir)

	class File:

		def __init__(self, canvas, x, y, w, h, name):
			self.x = x
			self.y = y
			self.width = w
			self.higth = h
			self.name = name

			self.rec = canvas.create_rectangle(x, y, w + x, h + y, fill="RED")
			self.text = canvas.create_text(x + w / 2, y + h + 15, text=self.name)

	class Folder:

		def __init__(self, canvas, x, y, w, h, name):
			self.x = x
			self.y = y
			self.width = w
			self.higth = h
			self.name = name

			self.rec = canvas.create_rectangle(x, y, w + x, h + y, fill="BLUE")
			self.text = canvas.create_text(x + w / 2, y + h + 15, text=self.name)
	

def main():
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
	client = c.Client("mc.gamingpassestime.com", 7000)
	
	window.read()
	gui = GUI(window["main"].TKCanvas)

	# Create an event loop
	while True:
		(method, data) = client.work()
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
				
				gui.draw_Directory(directory)

			print("Data")
		
		event, values = window.read()
		# End program if user closes window or
		# presses the OK button
		if event == "Exit" or event == sg.WIN_CLOSED:
			break
		else:
			payload = p.Payload(client.socket)
			if event == "send":
				payload.send_file("C:\\Users\\Navanda77\\school\\Filesystem\\Tests\\test.json", "test.json")
			elif event == "list":
				payload.get_list("root")
			else:
				print("Event: ", event)
				print("Values: ", values)
				window["date"].update("mamasd")

	window.close()


if __name__ == '__main__':
	main()