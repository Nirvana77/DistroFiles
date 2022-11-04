import socket
import os
import hashlib
import struct

def connect(host, port):
	s = socket.socket()  # instantiate
	s.connect((host, port))  # connect to the server
	s.settimeout(0.5)
	return s

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