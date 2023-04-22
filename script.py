import serial
import io
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

ser = serial.Serial(sys.argv[1], 115200, timeout = 0)
sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser))
sio.write("AT\r\n")
sio.flush()
queued.put(f"AT+ADDRESS={address}\r\n")
queued.put("AT+PARAMETER=12,4,1,7\r\n")

workbook = xlsxwriter.Workbook("data.xlsx")
worksheet = workbook.add_worksheet()

worksheet.clear()
worksheet.write("A1", "Temperature (°C)")
worksheet.write("B1", "Pressure (Pa)")
worksheet.write("C1", "Seismometer (g)")
worksheet.write("D1", "GPS Latitude")
worksheet.write("E1", "GPS Longitude")
worksheet.write("F1", "Time (ms)")
worksheet.write("G1", "Seismometer Time (ms)")

row = 2
seismorow = 2

ready = False
while True:
    #reading the data (output of arduino sketch)
    data = ser.readline()
    data = data.decode()
    data = data.strip()
    if data:
        ready = True
        end_time = time.time()
        elapsed_time = (end_time - start_time) * 1000 # in ms
        start_time = end_time
        print(data)

        data = data.split(",")
        nums = len(data) - 8

        worksheet.write(f"A{row}", data[2]) # Temperature
        worksheet.write(f"B{row}", data[3]) # Pressure

        for i in range(1, nums + 1): # Seismometer
            worksheet.write(f"C{seismorow}", data[3 + i])
            worksheet.write(f"G{seismorow}", elapsed_time * i / nums)
            seismorow += 1

        worksheet.write(f"D{row}", data[-4]) # GPS Latitude
        worksheet.write(f"E{row}", data[-3]) # GPS Longitude
        worksheet.write(f"F{row}", elapsed_time) # Time
        row += 1
        
    # if received data from other radio   
    if ready and not queued.empty():
        msg = queued.get()
        sio.write(msg)
        sio.flush()
        print("send", msg)
        ready = False
