import serial
import os
import datetime

today = datetime.datetime.now()

# create a file with today's date
newfile = str(today.year) + "_" + str(today.month) + "_" + str(today.day) + ".txt"

ser = serial.Serial("COM4", 9600) # change COM4 to whatever serial port you are using
print "connected to: " + ser.portstr
while True:
    data = ser.read()
    print data
    with open(newfile, "a+") as f:
        f.write(data)
