import serial
import sys
comPort = '/dev/ttyACM0'
logFile = 'flex.csv'
ser = serial.Serial(comPort, 9600, timeout=0)
ser.flushInput()

f = open(logFile, 'w')

print("Reading from: "+comPort)
print("Writing to  : "+logFile)
print("------------------------")

while True:
    try:
        byteIn = ser.read()
        if len(byteIn) != 0:
            f.write(byteIn.decode())
            sys.stdout.write(byteIn.decode())
            sys.stdout.flush()
    except:
        print('------------------------')
        print('Stopped')
        f.close()
        break
