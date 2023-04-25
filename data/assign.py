import xlsxwriter

workbook = xlsxwriter.Workbook("data.xlsx")
worksheet = workbook.add_worksheet()

worksheet.write("A1", "Temperature (°C)")
worksheet.write("B1", "Pressure (Pa)")
worksheet.write("C1", "Seismometer (gal)")
worksheet.write("D1", "GPS Latitude")
worksheet.write("E1", "GPS Longitude")
worksheet.write("F1", "Time (ms)")
worksheet.write("G1", "Seismometer Time (ms)")

with open("data.txt", "r") as f:
    lines = f.readlines()

row = 2
seismorow = 2

for l in lines:
    data = l.split(",")

    worksheet.write(f"A{row}", data[2]) # Temperature
    worksheet.write(f"B{row}", data[3]) # Pressure

    for i in range(1, int(data[-7]) + 1): 
        worksheet.write(f"C{seismorow}", data[3 + i]) # Seismometer
        worksheet.write(f"G{seismorow}", float(data[-6]) + (float(data[-5]) * i)) # Seismometer Time
        seismorow += 1

    worksheet.write(f"D{row}", data[-4]) # GPS Latitude
    worksheet.write(f"E{row}", data[-3]) # GPS Longitude
    worksheet.write(f"F{row}", data[-6]) # Time
    row += 1
    

workbook.close()
