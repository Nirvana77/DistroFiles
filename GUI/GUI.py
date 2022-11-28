import PySimpleGUI as sg
import os
import client as c
import payload as p
import memory as m
from threading import Thread
from urllib.parse import urlparse

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
	layouts = [
		[#* Connecting screen
			[
				sg.Text('Distributer'),
				sg.In(size=(25, 1), enable_events=True, key="-DIN-"),
				sg.Button("Connect to Distributer", key="Connect"),
			],
			[
				sg.Text('Server'),
				sg.In(size=(25, 1), enable_events=True, key="-SIN-"),
				sg.Text(':'),
				sg.In(size=(25, 1), enable_events=True, key="-PIN-"),
				sg.Button("Connect to Server", key="Connect"),
			]
		],
		[ #* Connected screen
			[
				sg.Canvas(key="main", size=(1000,500))
			],
			[
				sg.Button("Back"),
				sg.Text("File"),
				sg.In(size=(25, 1), enable_events=True, key="-TOUT-"),
				sg.FileBrowse(),
				sg.Button("Send", key="send"),
				sg.Button("Get list", key="list"),
			]
		]

	]

	connectionWindow = None
	connectedWindow = None
	currentWindow = None
	client = None
	canvas = None
	willStop = False

	icons = list()
	
	def __init__(self, onOpen = None, onClose = None, onEvent = None):
		self.onOpen = onOpen
		self.onClose = onClose
		self.onEvent = onEvent
		self.thread = Thread(target=self.work)
		
		# self.client = c.Client("mc.gamingpassestime.com", 7000, self.recv)

		# Create the window
		self.currentWindow = sg.Window("GUI", self.layouts[0])
		self.connectionWindow = self.currentWindow
		if not self.onOpen == None:
			self.onOpen(self, self.currentWindow)
		
		self.thread.start()

	def draw_Directory(self, list: list, path: str):
		if len(self.icons) > 0:
			last:self.Icon = self.icons[len(self.icons) - 1]
			(x, y) = (last.x + last.width + 10, last.y)
		else:
			(x, y) = (0,0)
		(w, h, margin) = (100, 100, 10)

		for i in self.icons:
			i.marked = False

		for d in list:
			if d["isFile"]:
				fileFolder = self.File(self, self.canvas, x, y, w, h, d["name"], path)
			else:
				fileFolder = self.Folder(self, self.canvas, x, y, w, h, d["name"], path)
				
			if not self.icons.__contains__(fileFolder):
				self.icons.append(fileFolder)
				x += w + margin
			else:
				fileFolder.destroy()
				for i in self.icons:
					if fileFolder.path == i.path and fileFolder.name == i.name:
						i.marked = True
		
		for i in self.icons:
			if not i.marked:
				i.destroy()
				self.icons.remove(i)

	def key(self, event):
		print("pressed: ", repr(event.char))

	def cliecked(self, event):
		for icon in self.icons:
			icon:self.Icon = icon
			if icon.x < event.x and icon.x + icon.width > event.x and icon.y < event.y and icon.y + icon.heigth > event.y:
				method, message = icon.click(event)
				""" if len(message) > 0:
					print("msg: ", message)
					data = p.messag_builder("", method, message)
					self.client.socket.sendall(data) """

	def connect(self, ip:str, port:int = None):
		self.client = c.Client(ip, port, self.recv)
		self.swapWindow(1)
		# self.client = c.Client("133.92.147.203", 8121, self.recv)
	def swapWindow(self, index:int):
		self.currentWindow.Hide()
		if index == 0:
			if self.connectionWindow == None:
				self.currentWindow = sg.Window("GUI", self.layouts[0])
				self.connectionWindow = self.currentWindow
			else:
				self.currentWindow = self.connectionWindow
				self.currentWindow.UnHide()
				if not self.client == None:
					self.client.destroy()
					self.client = None
			
		elif index == 1:
			if self.connectedWindow == None:
				self.currentWindow = sg.Window("GUI", self.layouts[1])
				self.connectedWindow = self.currentWindow
			else:
				self.currentWindow = self.connectedWindow
				self.currentWindow.UnHide()

	
	def closeWindows(self):
		if not self.connectedWindow == None:
			for i in self.icons:
				self.icons.remove(i)
			self.connectedWindow.close()
		
		if not self.connectionWindow == None:
			self.connectionWindow.close()
		

	def recv(self, method, data):
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
				
				self.draw_Directory(directory, "root")

	def uri_validator(self, x):
		try:
			result = urlparse(x)
			return all([result.netloc])
		except:
			return False

	def destroy(self, event:str, values):
		if not self.client == None:
			self.client.destroy()
			self.client = None
		
		self.closeWindows()
		self.onClose(self, event, values)

	def close(self):
		self.willStop = True

	def work(self):
		while not self.willStop:
			event, values = self.currentWindow.read()
			
			#if event == sg.WIN_CLOSED:
			#	self.onClose(self, event, values)
			if self.currentWindow == self.connectedWindow:
				self.connectedEvent(event, values)
			elif self.currentWindow == self.connectionWindow:
				self.connectionEvent(event, values)
			

		self.destroy(event, values)

	def connectionEvent(self, event:str, values):
		if event == "Exit" or event == sg.WIN_CLOSED:
			self.close()
		elif not event == None:
			if event.startswith("Connect"):
				dis:str = values["-DIN-"]
				serverIP:str = values["-SIN-"]
				try: 
					serverPort:int = int(values["-PIN-"])
				except:
					serverPort = None

				if serverIP.count(".") == 3 and not serverPort == None:
					self.connect(serverIP, serverPort)
				elif self.uri_validator(dis):
					self.connect(urlparse(dis).netloc, 7000)
				else:
					sg.popup("You need to enter a valed URL or IP with Port")

	def connectedEvent(self, event, values):
		if event == "Exit" or event == sg.WIN_CLOSED:
			self.close()
		elif event == "Back":
			self.swapWindow(0)
		elif self.canvas == None and not self.currentWindow["main"].TKCanvas == None:
			self.canvas = self.currentWindow["main"].TKCanvas
			self.canvas.bind("<Key>", self.key)
			self.canvas.bind("<Button-1>", self.cliecked)
			self.canvas.bind("<Button-3>", self.cliecked)

		if event == "send":
			path = self.currentWindow["-TOUT-"].get()
			self.sendPath(path)

		elif event == "list":
			msg = p.get_list("root")
			self.client.socket.sendall(p.messag_builder("", "list", msg))

		else:
			self.onEvent(self, event, values)

	def sendPath(self, path:str):
		name = ""
		ext = ""
		
		for i in reversed(range(0, len(path))):
			if path[i] == '/' or path[i] == '\\':
				break
			else:
				name += path[i]
			if len(ext) == 0 or not ext[len(ext) - 1] == '.':
				ext += path[i]

		name = name[::-1]
		ext = ext[::-1]
		msg = p.send_file(path, name)
		self.client.socket.sendall(p.messag_builder("", "upload", msg))


	class Icon:

		def __init__(self, gui, canvas, x: int, y: int, w: int, h: int, name: str, path: str):
			self.gui = gui
			self.x = x
			self.y = y
			self.width = w
			self.heigth = h
			self.name = name
			self.canvas = canvas
			self.path = path
			self.marked = True

			self.rec = canvas.create_rectangle(x, y, w + x, h + y)
			self.text = canvas.create_text(x + w / 2, y + h + 15, text=self.name)

		def destroy(self):
			self.canvas.delete(self.rec)
			self.canvas.delete(self.text)

		def __eq__(self, __o: object) -> bool:
			if __o.__class__ is not self.__class__:
				return False

			return self.name == __o.name

		def __str__(self) -> str:
			return f"Icon: {self.name}"

		def click(self, event) -> list:
			print("Clicked at, ", self.name, " at (", event.x, ", ", event.y, ")")
			return list()
	
	class File(Icon):

		def __init__(self, gui, canvas, x: int, y: int, w: int, h: int, name: str, path: str):
			super().__init__(gui, canvas, x, y, w, h, name, path)
			self.canvas.itemconfig(self.rec, fill="RED")

		def click(self, event) -> list:
			if event.num == 1:
				print("Left Click File")
				method = "get"
				
				path = self.path
				if not path[len(path) - 1] == '/' and not path[len(path) - 1] == '\\':
					path += "/"
				
				path += self.name

				msg = [True, int(len(path)/256), len(path)%256]
				for i in list(path.encode('ascii')):
					msg.append(i)

				return (method, msg)
			elif event.num == 3:
				print("Rigth Click File")
				self.gui.client.delete(self.path + "/" + self.name)

			return list()

	class Folder(Icon):

		def __init__(self, gui, canvas, x: int, y: int, w: int, h: int, name: str, path: str):
			super().__init__(gui, canvas, x, y, w, h, name, path)
			self.canvas.itemconfig(self.rec, fill="BLUE")
		
		def click(self, event) -> list:
			if event.num == 1:
				print("Left Click Folder")
				
				path = self.path
				if not path[len(path) - 1] == '/' and not path[len(path) - 1] == '\\':
					path += "/"
				
				path += self.name
				
				msg = p.get_list(path)

				return ("list", msg)
			elif event.num == 3:
				print("Rigth Click Folder")
			return list()

def main():
	try:
		os.remove("file_dump.txt")
	except os.error as e:
		err = e.args[0]
		if not err == 2:
			print(e)
	
	GUI(onOpen, onClose, onEvent)

def onOpen(gui: GUI, window: sg.Window):
	print("onOpen")
	
def onClose(gui: GUI, event: str, values: list):
	print("onClose")
	
def onEvent(gui: GUI, event: str, values: list):
	print("onEvent")
	print("Event: ", event)
	print("Values: ", values)

if __name__ == '__main__':
	main()