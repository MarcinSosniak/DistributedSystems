import socket
import time
import sys
import struct
import os

file_name="default_logger.txt"

#   224.1.1.1 5007
mcast_group = '224.1.1.1'
mcast_port = 5007
windows = True # niestety konkretny bind() do multicastowego addresu ip nie dziala w windowsach
#internet milczy jesli chodzi o rozwiazanie 

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
if windows:
    sock.bind(('', mcast_port))
else:
    sock.bind((mcast_group, mcast_port))
mreq = struct.pack("4sl", socket.inet_aton(mcast_group), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

if len(sys.argv) >= 2 :
    file_name = sys.argv[1]

with open(file_name,"a") as file:
    while True:
        msg = sock.recv(1024)
        msg_str=msg.decode('ascii')
        words_list=msg_str.split()
        time_val=float(words_list[0])
        words_list[0]=time.ctime(time_val)
        nice_date_and_name=words_list[0]+" "+words_list[1]
        print(nice_date_and_name)
        file.write(nice_date_and_name + "\n")
        file.flush()
        os.fsync(file.fileno())