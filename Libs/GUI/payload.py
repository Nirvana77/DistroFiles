import hashlib
import memory as m
import struct

def send_file(filepath, filename):
	# file = input("filename > ")

	message = [1, int(len(filename)/256), len(filename)%256]

	for i in list(filename.encode('ascii')):
		message.append(i)

	file_arr = load_file(filepath)
	file_hash_arr = str_to_hex_array(hash_file(filepath))

	length = len(file_arr)
	
	print(file_arr)

	message.append(int(length/256))
	message.append(length%256)
	for i in file_arr:
		message.append(i)

	message = message + file_hash_arr
	return message

def get_list(filepath) -> list:
	message = [int(len(filepath)/256), len(filepath)%256]
	for i in list(filepath.encode('ascii')):
		message.append(i)

	return message

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

	file_bytes = load_file(filename)

	for i in range(0, len(file_bytes), 8):
		chunk = file_bytes[i:i + 8]
		h.update(chunk)
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

def get_crc(array, result = 0) -> int:
	CRC = 0b01011

	for data in array:
		if data > 0xff:
			print(data)

		result = result + ((data & 0xff) << 4)
		for j in reversed(range(4, 12)):
			bit = (result >> j) & 0x1 

			if bit > 1:
				print("Bit error")

			result = result & ~(0x1 << j)
			if(bit == 1):
				result = result ^ (CRC << (j - 4))
		if result > 0xf:
			print("result error")

	if not result == 0x8:
		print("error")
	
	return result

def messag_builder(src, des, method, message) -> bytearray:
	array = bytearray()
	flag = 0
	if(method != ""):
		flag += 1 << 2

	if(des != ""):
		flag += 1 << 1

	if(src != ""):
		flag += 1 << 0
		
	for v in [flag, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16, 0, 1,2,3,4,5,6,7,8]:
		array.append(v)
	
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
		
	elif (type(message) is list or type(message) is bytearray):
		for v in message:
			array.append(v)
		
	crc = get_crc(array)

	array.append(crc)

	return array

def load_file(filename):
	""" text_file = open(filename, "r")
	lines = text_file.readlines()
	print(lines)
	print(len(lines))
	text_file.close() """
	file = open(filename,'rb')
	lines = file.read()
	file.close()
	ext = ""

	for i in reversed(range(0, len(filename))):
		
		if len(ext) == 0 or not ext[len(ext) - 1] == '.':
			ext += filename[i]
		elif ext[len(ext) - 1] == '.':
			break
	
	if ext == ".txt" or ext == ".json":
		i = 0
		for v in lines:
			if v == 0xd:
				lines = lines[:i] + lines[i+1:]
				i -= 1
			i += 1
	
	return lines

def remove_bytes(buffer, start, end):
    fmt = '%ds %dx %ds' % (start, end-start, len(buffer)-end)  # 3 way split
    return b''.join(struct.unpack(fmt, buffer))

def recive(msg):
	data = byte_to_Array(msg)
	print("data: ", data)
	(i, flag) = m.get_UInt8(data, 0)

	uuid = []
	for x in range(i, i + 16):
		uuid.append(data[x])
		i+=1
	i += 1 + 8
	
	i += 1
	src = ""
	for x in range(i, i + 6):
		src += str(data[x]) + " "
		i+=1
	i += 1
	des = ""
	for x in range(i, i + 6):
		des += str(data[x]) + " "
		i+=1
	
	i+=1
	(i, size) = m.get_UInt16(data, i)
	
	method = ""
	if flag >> 2 & 0x1 == 1:
		for x in range(i, i + size):
			method += chr(data[x])
		i += size
		(i, size) = m.get_UInt16(data, i)
	
	message = []
	if size != 0:
		message = data[i:i + size]

	return (uuid, src, des, method, message)