import PySimpleGUI as sg
import client as c
import payload as p
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
		self.canvas.bind("<Key>", self.key)
		self.canvas.bind("<Button-1>", self.cliecked)
		self.client = c.Client("mc.gamingpassestime.com", 7000, self.recv, self)

	def draw_Directory(self, list: list, path: str):
		if len(self.map) > 0:
			(x, y) = (self.map[len(self.map) - 1].x, self.map[len(self.map) - 1].y)
		else:
			(x, y) = (0,0)
		(w, h, margin) = (100, 100, 10) 

		for d in list:
			if d["isFile"]:
				fileFolder = self.File(self.canvas, x, y, w, h, d["name"], path)
			else:
				fileFolder = self.Folder(self.canvas, x, y, w, h, d["name"], path)
				
			if not self.map.__contains__(fileFolder):

				self.map.append(fileFolder)
				x += w + margin
			else:
				fileFolder.destroy()

	def key(self, event):
		print("pressed: ", repr(event.char))

	def cliecked(self, event):
		for icon in self.map:
			if icon.x < event.x and icon.x + icon.width > event.x and icon.y < event.y and icon.y + icon.heigth > event.y:
				icon.click(event)

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

			print("Data")
	class Icon:

		def __init__(self, canvas, x: int, y: int, w: int, h: int, name: str):
			self.x = x
			self.y = y
			self.width = w
			self.heigth = h
			self.name = name
			self.canvas = canvas

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

		def click(self, event):
			print("Clicked at, ", self.name)
	
	class File(Icon):

		def __init__(self, canvas, x: int, y: int, w: int, h: int, name: str, path: str):
			super().__init__(canvas, x, y, w, h, name)
			self.path = path
			self.canvas.itemconfig(self.rec, fill="RED")

		def click(self, event):
			print("Clicked File")
	class Folder(Icon):

		def __init__(self, canvas, x: int, y: int, w: int, h: int, name: str, path: str):
			super().__init__(canvas, x, y, w, h, name)
			self.path = path
			self.canvas.itemconfig(self.rec, fill="BLUE")
		
		def click(self, event):
			print("Clicked Folder")

def main():
	layout = [
		[
			sg.Canvas(key="main", size=(1000,500))
		],
		[
			sg.Button("Exit"),
			sg.Text("Image File"),
			sg.In(size=(25, 1), enable_events=True, key="-FOLDER-"),
			sg.FileBrowse(),
			sg.Button("Get list", key="list"),
		]
	]


	# Create the window
	gui = None
	client = None
	window = sg.Window("GUI", layout)

	while True:
		event, values = window.read()

		if gui == None and not window["main"].TKCanvas == None:
			gui = GUI(window["main"].TKCanvas)
			client = gui.client
		
		# End program if user closes window or
		# presses the OK button
		if event == "Exit" or event == sg.WIN_CLOSED:
			break
		
		if event == "-FOLDER-":
			file = values["-FOLDER-"]
			print("Event: ", event)
			print("Values: ", values)
			print("File: ", file)

		else:
			payload = p.Payload(client.socket)
			if event == "send":
				payload.send_file("C:\\Users\\Navanda77\\school\\Filesystem\\Tests\\test.json", "test.json")
			elif event == "list":
				payload.get_list("root")
			else:
				print("Event: ", event)
				print("Values: ", values)

	window.close()
	client.destroy()


if __name__ == '__main__':
	main()