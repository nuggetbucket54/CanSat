import serial
import io
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import xlsxwriter
import sys
from queue import Queue

print(sys.argv)
queued = Queue()

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
worksheet.write("A1", "Temperature")
worksheet.write("B1", "Pressure")
worksheet.write("C1", "GPS")
worksheet.write("D1", "Seismometer")
row = 2

ready = False
while True:
    #reading the data (output of arduino sketch)
    data = ser.readline()
    data = data.decode()
    data = data.strip()
    if data:
        ready = True
        print(data)
        data = data.split(",")
        worksheet.write("A" + str(row), data[0]) # Temperature
        worksheet.write("B" + str(row), data[1]) # Pressure
        worksheet.write("C" + str(row), data[2]) # GPS
        worksheet.write("D" + str(row), data[3]) # Seismometer
        row += 1
        # if received data from other radio
        
    if ready and not queued.empty():
        msg = queued.get()
        sio.write(msg)
        sio.flush()
        print("send", msg)
        ready = False

"""
while True:
    try:
        #reading the data (output of arduino sketch)
        data = ser.readline()
        data = data.decode()
        data = data.strip()
        worksheet.write("A" + str(row), data)
        row += 1
    except:
        pass

x_vals = []
y_vals = []

def animate(i):
    try:
        #reading the data (output of arduino sketch)
        data = ser.readline()
        data = data.decode()
        data = data.strip()

        #calculating time elapsed since start of program
        current_t = datetime.datetime.now()
        elapsed_t = (current_t - initial_t).total_seconds() * 1000

        x_vals.append(elapsed_t)
        y_vals.append(data)

        plt.cla()

        plt.plot(x_vals, y_vals)        
        #print(*[x, y, z])

    except:
        pass


plt.style.use('fivethirtyeight')

ani = FuncAnimation(plt.gcf(), animate, interval=500)

plt.tight_layout()
plt.show()

# def animate(i):
#     data = pd.read_csv('data.csv')
#     x = data['x_value']
#     y1 = data['total_1']
#     y2 = data['total_2']

#     plt.cla()

#     plt.plot(x, y1, label='Channel 1')
#     plt.plot(x, y2, label='Channel 2')

#     plt.legend(loc='upper left')
#     plt.tight_layout()
"""
