import socket
import os

def get_crc(array):
	result = 0
	CRC = 0b01011

	for data in array:
		result = result + (data << 4)
		for j in reversed(range(5, 12)):
			bit = result >> j
			result = result  & ~(0x1 << j)
			if(bit == 1):
				result = result ^ (CRC << (j - 4))

	return result

def messag_builder(src, des, method, message):
	array = []
	flag = 0
	if(method != ""):
		flag += 1 << 2

	if(des != ""):
		flag += 1 << 1

	if(src != ""):
		flag += 1 << 0
		
	array = [flag, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16, 0, 1,2,3,4,5,6,7,8]
	
	if(src != ""):
		array.append(2)
		for x in range(1,7):
			array.append(x)
	else:
		array.append(0)
		for x in range(1,7):
			array.append(0)

	if(des != ""):
		array.append(2)
		for x in range(1,7):
			array.append(x)
	else:
		array.append(0)
		for x in range(1,7):
			array.append(0)

	if(method != ""):
		array.append(1)
		length = len(method)
		array.append(int(length/256))
		array.append(length%256)
		for i in list(method.encode('ascii')):
			array.append(i)
	
	length = len(message)
	array.append(int(length/256))
	array.append(length%256)

	if(type(message) is str):
		for i in list(message.encode('ascii')):
			array.append(i)
	elif (type(message) is list):
		array = array + message
		
	crc = get_crc(array)
	array.append(crc)

	return array

def load_file(filename):
	text_file = open(filename, "r")
	lines = text_file.readlines()
	print(lines)
	print(len(lines))
	text_file.close()

	return lines

def client_program():
	host = socket.gethostname()  # as both code is running on same pc
	port = 8021  # socket server port number

	client_socket = socket.socket()  # instantiate
	client_socket.connect((host, port))  # connect to the server

	data = ""
	method = ""
	message = ""
	while data != "q":
		os.system('cls')
		print("1) Upload file")
		print("2) Remove file")
		print("3) Rename file")

		data = input('method -> ')

		if(data == "1"):
			method = "upload"
			os.system('cls')
			file = input("filename > ")

			message = [1, int(len("settings.json")/256), len("settings.json")%256]
			
			for i in list("settings.json".encode('ascii')):
				message.append(i)
			
			file_arr = load_file("/home/navanda/github/Filesystem/Shared/settings.json")
			
			file_str = ""
			file_str = file_str.join(file_arr)
			length = len(file_str)

			message.append(int(length/256))
			message.append(length%256)
			for i in list(file_str.encode('ascii')):
				message.append(i)
			
			data = "s"

		elif(data != "q"):
			data = ""
			

		if(data == "s"):
			arr = messag_builder("1", "", method, message)
			print(arr)
			
			client_socket.send(bytearray(arr))
	

	client_socket.close()  # close the connection


if __name__ == '__main__':
	client_program()