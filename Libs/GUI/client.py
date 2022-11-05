import socket
import os
import payload as p

class Client:

	def __init__(self, host, port) -> None:
		self.socket = socket.socket()  # instantiate
		self.socket.connect((host, port))  # connect to the server
		self.socket.settimeout(0.5)
		
	def work(self):
		
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
				return (method, message)
		return ("", [])
