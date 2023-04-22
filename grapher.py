import random
import xlsxwriter
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import pandas as pd

workbook = xlsxwriter.Workbook("data.xlsx")
worksheet = workbook.add_worksheet()

df = pd.read_excel('data.xlsx', names=['A','B','C','D','E','F','G'])
print(df.columns)
plt.plot(df['G'].values, df['C'].values)
plt.xlabel('Time (ms)')
plt.ylabel('Acceleration (Gs)')
plt.show()

