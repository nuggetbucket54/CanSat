import serial
import datetime
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import xlsxwriter

ser = serial.Serial('/dev/cu.usbserial-141330', 115200, timeout = 0.5)
"""
initial_t = datetime.datetime.now()
workbook = xlsxwriter.Workbook("data.xlsx")
worksheet = workbook.add_worksheet()
row = 1
"""

while True:
    try:
        #reading the data (output of arduino sketch)
        data = ser.readline()
        data = data.decode()
        data = data.strip()
        print(data)
    except:
        pass

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