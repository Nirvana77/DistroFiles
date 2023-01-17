from time import sleep
import socket
import payload as p
from threading import Thread

class Client:

	def __init__(self, host, port, callback) -> None:
		self.socket = socket.socket()  # instantiate
		self.socket.settimeout(0.5)
		self.thread = Thread(target=self.work)
		self.callback = callback
		self.willStop = False
		self.connected = False
		self.host = host
		self.port = port

		self.thread.start()

	def connect(self):
		try:
			self.socket.connect((self.host, self.port))  # connect to the server
			self.socket.sendall(p.messag_builder("", "discover", list(), willPrint=True))

			msg = bytes()
			while len(msg) == 0:
				msg = self.socket.recv(4096)

			print("connect msg: ", msg)
				
		except socket.timeout as e:
			err = e.args[0]
			# this next if/else is a bit redundant, but illustrates how the
			# timeout exception is setup
			if err != 'timed out':
				print("connect timeout error: ", e)
				return
		except socket.error as e:
			err = e.args[0]
			# Something else happened, handle error, exit, etc.
			if err == 10053 or err == 10056 or err == 10054:
				self.socket.detach()
				self.socket = socket.socket()
				self.socket.settimeout(0.5)
				self.connected = False
				sleep(3)
				return
			else:
				print("connect error: ", e)
				return
		
		self.connected = True

	def delete(self, path:str):
		message = list()

		message.append(len(path))
		for i in list(path.encode('ascii')):
			message.append(i)

		arr = p.messag_builder("", "delete", message)
		self.socket.sendall(arr)

	def work(self):

		while (not self.willStop):
			if not self.connected:
				self.connect()

			else:
				try:
					msg = self.socket.recv(4096)
					while len(msg) % 4096 == 0 and not len(msg) == 0:
						msg = msg + self.socket.recv(4096)
						
				except socket.timeout as e:
					err = e.args[0]
					# this next if/else is a bit redundant, but illustrates how the
					# timeout exception is setup
					if err != 'timed out':
						print(e)
				except socket.error as e:
					err = e.args[0]
					if err == 10053:
						self.socket.detach()
						self.socket = socket.socket()
						self.connect()
					# Something else happened, handle error, exit, etc.
					else: 
						print(e)
				else:
					if not len(msg) == 0:
						(uuid, src, des, method, message) = p.recive(msg, willPrint = True)
						self.callback(method, message)
					
		self.socket.close()
	
	def destroy(self):
		self.willStop = True
