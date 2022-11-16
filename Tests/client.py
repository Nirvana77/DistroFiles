import socket
import os
import hashlib

def hash_file(filename):
   """"This function returns the SHA-1 hash
   of the file passed into it"""

   # make a hash object
   h = hashlib.md5()

   # open file for reading in binary mode
   with open(filename,'rb') as file:

       # loop till the end of the file
       chunk = 0
       while chunk != b'':
           # read only 1024 bytes at a time
           chunk = file.read(1024)
           h.update(chunk)

   # return the hex representation of digest
   return h.hexdigest()

def str_to_hex_array(string):
	arr = []

	string_arr = list(string.encode('ascii'))

	i = 0
	value = 0
	for c in string_arr:
		v = c - 48
		if(v > 9):
			v = 10 + c - 97 
		value = (value << 4) + v
		if(i % 2 == 1):
			arr.append(value)
			value = 0
		i = i + 1


	return arr

# TODO: #49 fix crc
def get_crc(array, result = 0):
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
	array = [[]]
	index = 0
	flag = 0
	if(method != ""):
		flag += 1 << 2

	if(des != ""):
		flag += 1 << 1

	if(src != ""):
		flag += 1 << 0
		
	array[index] = [flag, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16, 0, 1,2,3,4,5,6,7,8]
	
	if(src != ""):
		array[index].append(2)
		for x in range(1,7):
			array[index].append(x)
	else:
		array[index].append(0)
		for x in range(1,7):
			array[index].append(0)

	if(des != ""):
		array[index].append(2)
		for x in range(1,7):
			array[index].append(x)
	else:
		array[index].append(0)
		for x in range(1,7):
			array[index].append(0)

	if(method != ""):
		array[index].append(1)
		length = len(method)
		array[index].append(int(length/256))
		array[index].append(length%256)
		for i in list(method.encode('ascii')):
			array[index].append(i)
	
	length = len(message)
	array[index].append(int(length/256))
	array[index].append(length%256)

	if(length + len(array[index]) >= 300):
		index = index + 1
		array.append([])

	if(type(message) is str):
		for i in list(message.encode('ascii')):
			array[index].append(i)
		
	elif (type(message) is list):
		array[index] = array[index] + message
		
	crc = 0
	for arr in array:
		crc = get_crc(arr, crc)

	array[index].append(crc)

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
		os.system('clr')
		print("1) Upload file")
		print("2) Remove file")
		print("3) Rename file")

		data = input('method -> ')

		if(data == "1"):
			method = "upload"
			os.system('clear')
			# file = input("filename > ")
			file = "/home/navanda/github/Filesystem/Shared/settings.json"

			message = [1, int(len("settings.json")/256), len("settings.json")%256]
			
			for i in list("settings.json".encode('ascii')):
				message.append(i)
			
			file_arr = load_file(file)
			file_hash_arr = str_to_hex_array(hash_file(file))
			
			file_str = ""
			file_str = file_str.join(file_arr)
			length = len(file_str)

			message.append(int(length/256))
			message.append(length%256)
			for i in list(file_str.encode('ascii')):
				message.append(i)

			message = message + file_hash_arr
			
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