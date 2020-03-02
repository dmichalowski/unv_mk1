import serial
import time
from threading import Lock

class UartCommunication:
    def __init__(self,port):
        self.lock = Lock()
        self.sendSerial = serial.Serial (port, 9600)
        self.readoutSerial = serial.Serial (port, 9600)
        time.sleep(2)

    def readPackage(self):
        #print("flag 1")
        if(self.readoutSerial.read() == b'\xff'):
            #print("flag 2")
            header = self.readoutSerial.read(3)
            #print("flag 3")
            data = self.readoutSerial.read(header[2])
            #print("flag 4")
            if(len(data) > 5):
                self.readoutSerial.flushInput()
                return None
        else:
            self.readoutSerial.flushInput()
            return None
        return (header + data)

    def writePackage(self,data):
        self.lock.acquire()
        try:
            self.sendSerial.write(data)
            #time.sleep(1)
        except Exception as e:
            print(e)
            pass
        finally:
            self.lock.release()
            
def main():
    uart = UartCommunication('/dev/ttyUSB1')
    serialMsg = bytes([255,4,3,2,43,43])
    while True:
        if uart.readoutSerial.inWaiting():
            byte = uart.readPackage()
            print(byte)


if __name__ == '__main__':
    main()

    
