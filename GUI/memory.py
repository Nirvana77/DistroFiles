
def get_UInt8(data, i):
	return (i + 1, int(data[i]))

def get_UInt16(data, i):
	return (i + 2, int(data[i]/256 + data[i + 1]))
def get_Seting(data, i, size):
	string = ""
	for x in range(i, i + size):
		string += chr(data[x])

	return (i + size, string)