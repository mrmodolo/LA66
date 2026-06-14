import serial
import time

ser = serial.Serial("/dev/ttyUSB0", 115200)

print("DTR ON")
ser.setDTR(True)
time.sleep(2)

print("DTR OFF")
ser.setDTR(False)
time.sleep(2)

print("RTS ON")
ser.setRTS(True)
time.sleep(2)

print("RTS OFF")
ser.setRTS(False)

ser.close()
