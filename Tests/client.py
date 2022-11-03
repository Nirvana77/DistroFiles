import socket

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
	
	if(message != ""):
		length = len(message)
		array.append(int(length/256))
		array.append(length%256)
		for i in list(message.encode('ascii')):
			array.append(i)
	crc = get_crc(array)
	array.append(crc)

	return array

def client_program():
	host = socket.gethostname()  # as both code is running on same pc
	port = 8021  # socket server port number

	client_socket = socket.socket()  # instantiate
	client_socket.connect((host, port))  # connect to the server

	method = input('method -> ')
	message = input('message -> ')
	arr = messag_builder("1", "", method, message)
	print(arr)
	# data_string = pickle.dumps(arr)
	# client_socket.send(data_string)
	client_socket.send(bytearray(arr))
	# myArrayString = ""
	# for item in arr:
	#     print("item: ", item)
	#     myArrayString= myArrayString+ str(item)
	# print(myArrayString)
	# client_socket.send((myArrayString).encode())
	

	client_socket.close()  # close the connection


if __name__ == '__main__':
	client_program()