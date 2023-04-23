import matplotlib.pyplot as plt
import pandas as pd
from scipy.signal import lfilter

df = pd.read_excel('data.xlsx', names=['A','B','C','D','E','F','G','H','I','J'])

temperature = df['A'].values
pressure = df['B'].values
seismometer = df['C'].values
gpslat = df['D'].values
gpslon = df['E'].values
time_ms = df['F'].values
seismotime_ms = df['G'].values
time_s = df['H'].values
seismotime_s = df['I'].values
altitude = df['J'].values

n = 15
b = [1.0 / n] * n
a = 1
filter_seismo = lfilter(b, a, seismometer)

fig, ax1 = plt.subplots()
ax1.plot(time_s, altitude, color='blue')
ax1.set_xlabel('Time (s)')
ax1.set_title("Altitude (m) vs. Time (s)")
ax1.set_ylabel('Altitude (m)')
ax1.tick_params(axis='y')

ax2 = ax1.twinx()
ax2.plot(time_s, temperature, color='red')
ax2.set_ylabel('Temperature (°C)', color='red')
ax2.tick_params(axis='y', labelcolor='red')


# ax3 = ax1.twinx()
# ax3.spines["right"].set_position(("axes", 1.2))
# ax3.plot(time_s, altitude, color='green')
# ax3.set_ylabel('Altitude (m)', color='green')
# ax3.tick_params(axis='y', labelcolor='green')

plt.figure()
plt.plot(altitude, temperature)
plt.title("Temperature (°C) vs. Altitude (m)", fontsize=16)
plt.xlabel("Altitude (m)", fontsize=12)
plt.ylabel("Temperature (°C)", fontsize=12)

plt.figure()
plt.plot(time_s, pressure)
plt.title("Pressure (Pa) vs time (s)", fontsize=16)
plt.xlabel("Time(s)", fontsize=12)
plt.ylabel("Pressure (Pa)", fontsize=12)

plt.figure()
plt.plot(altitude, temperature)
plt.title("Pressure (Pa) vs altitude (m)", fontsize=16)
plt.xlabel("Altitude (m)", fontsize=12)
plt.ylabel("Pressure (Pa)", fontsize=12)

plt.figure()
plt.plot(time_s, altitude)
plt.plot(time_s, temperature)
plt.plot(time_s, pressure)

# plt.figure()
# plt.plot(temperature, pressure)
# plt.title("Pressure (Pa) vs. Temperature (°C)")

plt.figure()
plt.plot(seismotime_s, filter_seismo)
plt.title ("Seismometer data vs. time (s)", fontsize=16)
plt.xlabel("Time (s)", fontsize=12)
plt.ylabel("Seismometer reading (gal (0.01m/s^2))", fontsize=12)

plt.show()

