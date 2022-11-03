import socket

def message(message):
    return 0

def client_program():
    host = socket.gethostname()  # as both code is running on same pc
    port = 8021  # socket server port number

    client_socket = socket.socket()  # instantiate
    client_socket.connect((host, port))  # connect to the server



    client_socket.close()  # close the connection


if __name__ == '__main__':
    client_program()