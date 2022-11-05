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

def client_program():
		client_socket = connect(socket.gethostname(), 8021)
		data = ""
		method = ""
		message = ""
		while data != "q":
			os.system('clr')
			print("1) Upload file")
			print("2) Remove file")
			print("3) Rename file")

			data = input('-> ')

			if(data == "1"):
				send_file(client_socket, "setting.json")

			elif(data != "q"):
				data = ""
				

			if(data == "s"):
				arr = messag_builder("1", "", method, message)
				print(arr)
				
				client_socket.send(bytearray(arr))
		

		client_socket.close()  # close the connection


if __name__ == '__main__':
	client_program()