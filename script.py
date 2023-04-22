import serial
import io
import datetime
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import xlsxwriter
import sys
from queue import Queue
import time

print(sys.argv)
queued = Queue()
start_time = time.time()


if len(sys.argv) < 2:
    print("Usage: python3 script.py <port> <address=5656> <networkID=12> <passphrase=none>. Replacing a field with D will use the default value.")
    exit()

address = 5656
networkID = 12
if len(sys.argv) > 2 and sys.argv[2] != "D":
    print(sys.argv[2])
    address = int(sys.argv[2])
if len(sys.argv) > 3 and sys.argv[3] != "D":
    networkID = int(sys.argv[3]);
if len(sys.argv) > 4:
    queued.put(f"AT+CPIN={str(sys.argv[4]).zfill(32)}\r\n")

ser = serial.Serial(sys.argv[1], 38400, timeout = 0.20)
sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser))
sio.write("AT\r\n")
sio.flush()
queued.put(f"AT+ADDRESS={address}\r\n")
#queued.put("AT+PARAMETER=12,4,1,7\r\n")

ready = False
tflag = True
with open('data.txt', 'w') as f:
    while True:
        #reading the data (output of arduino sketch)
        data = ser.readline()
        data = data.decode()
        data = data.strip()

        if not data.startswith("+RCV"):
            pass
        else:
            if data:
                ready = True
                end_time = time.time()
                elapsed_time = (end_time - start_time) * 1000 # in ms
                if tflag:
                    timediff = elapsed_time
                    last_time = end_time
                    tflag = False
                else:
                    timediff = (end_time - last_time) * 1000
                    last_time = end_time

                print(data)

                data = data.split(",")
                num = len(data)-8
                data.insert(-4, str(num))
                data.insert(-4, str(elapsed_time))
                data.insert(-4, str(timediff / num))
                f.write(",".join(data) + "\n")
                
            # if received data from other radio   
            if ready and not queued.empty():
                msg = queued.get()
                sio.write(msg)
                sio.flush()
                print("send", msg)
                ready = False
