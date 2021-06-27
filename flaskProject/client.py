from socket import *


class Client:
    def __init__(self):
        self.tcp_client_socket = None

    def connect(self, ipaddr):
        HOST = ipaddr
        PORT = 333  # ESP8266默认端口
        ADDRESS = (HOST, PORT)
        self.tcp_client_socket = socket(AF_INET, SOCK_STREAM)
        self.tcp_client_socket.connect(ADDRESS)

    def send_msg(self, msg):
        data = msg
        self.tcp_client_socket.send(data.encode('utf-8'))

    def deconnect(self):
        self.tcp_client_socket.close()


def main():
    c = Client()
    c.connect('192.168.2.246')
    c.send_msg('nb')
    c.deconnect()


if __name__ == '__main__':
    main()
