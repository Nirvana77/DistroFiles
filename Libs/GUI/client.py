import socket
import os
import payload as p
from threading import Thread

class Client:

	def __init__(self, host, port, callback) -> None:
		self.socket = socket.socket()  # instantiate
		self.socket.settimeout(0.5)
		self.thread = Thread(target=self.work)
		self.callback = callback
		self.willStop = False
		self.host = host
		self.port = port
		self.connect()

		self.thread.start()

	def connect(self):
		self.socket.connect((self.host, self.port))  # connect to the server
		
	def work(self):
		while (not self.willStop):
		
			try:
				msg = self.socket.recv(4096)
			except socket.timeout as e:
				err = e.args[0]
				# this next if/else is a bit redundant, but illustrates how the
				# timeout exception is setup
				if err != 'timed out':
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
					self.callback(method, message)
	
	def destroy(self):
		self.willStop = True
		self.thread.join()
