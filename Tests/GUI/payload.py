import socket
import os
import hashlib
import struct


def byte_to_Array(str):
	arr = []

	for c in str:
		arr.append(c)

	return arr


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

def send_file(s, filepath, filename):
	method = "upload"
	# file = input("filename > ")

	message = [1, int(len(filename)/256), len(filename)%256]

	for i in list(filename.encode('ascii')):
		message.append(i)

	file_arr = load_file(filepath)
	file_hash_arr = str_to_hex_array(hash_file(filepath))

	file_str = ""
	file_str = file_str.join(file_arr)
	length = len(file_str)

	message.append(int(length/256))
	message.append(length%256)
	for i in list(file_str.encode('ascii')):
		message.append(i)

	message = message + file_hash_arr
	arr = messag_builder("1", "", method, message)
	print(arr)
	
	s.send(bytearray(arr[0]))

def get_list(s, filepath):
	method = "list"
	message = [int(len(filepath)/256), len(filepath)%256]
	for i in list(filepath.encode('ascii')):
		message.append(i)

	arr = messag_builder("1", "", method, message)
	print(arr)
	
	s.send(bytearray(arr[0]))

def recive(msg):
	data = byte_to_Array(msg)
	print("data: ", data)
	i = 0
	flag = data[0]
	i+=1

	uuid = []
	for x in range(i, i + 16):
		uuid.append(data[x])
		i+=1
	i += 1 + 8
	
	i += 1
	src = []
	for x in range(i, i + 6):
		src.append(data[x])
		i+=1
	i += 1
	des = []
	for x in range(i, i + 6):
		des.append(data[x])
		i+=1
	
	i+=1
	size = data[i]*256 + data[i + 1]
	i += 2
	method = ""
	if flag >> 2 & 0x1 == 1:
		for x in range(i, i + size):
			method += chr(data[x])
		i += size
		size = data[i]*256 + data[i + 1]
		i += 2
	
	message = []
	if size != 0:
		message = data[i:i + size]

	return (uuid, src, des, method, message)

	text_file = open(filename, "r")
	lines = text_file.readlines()
	print(lines)
	print(len(lines))
	text_file.close()

	return lines

def send_file(s, filepath, filename):
	method = "upload"
	# file = input("filename > ")

	message = [1, int(len(filename)/256), len(filename)%256]

	for i in list(filename.encode('ascii')):
		message.append(i)

	file_arr = load_file(filepath)
	file_hash_arr = str_to_hex_array(hash_file(filepath))

	file_str = ""
	file_str = file_str.join(file_arr)
	length = len(file_str)

	message.append(int(length/256))
	message.append(length%256)
	for i in list(file_str.encode('ascii')):
		message.append(i)

	message = message + file_hash_arr
	arr = messag_builder("1", "", method, message)
	print(arr)
	
	s.send(bytearray(arr[0]))
