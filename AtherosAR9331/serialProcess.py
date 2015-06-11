#
# Class to send and receive messages to/from the serial port connecting the 
# Atheros AR9331 Linux System on the Arduino Yun with the Arduino compatible
# ATmega microcontroller located on the same board. The commands to sent to
# the ATmega microcontroller are read from the task queue (taskQ). Commands
# received from the microcontroller are written to the result queue 
# (resultQ).
#
import serial
import time
import multiprocessing

class SerialProcess(multiprocessing.Process):

    def __init__(self, taskQ, resultQ):
        multiprocessing.Process.__init__(self)
        self.taskQ = taskQ
        self.resultQ = resultQ
        self.sp = serial.Serial(port='/dev/ttyATH0', baudrate=115200, timeout=2)

    def close(self):
        self.sp.close()

    def sendData(self, data):
        self.sp.write(data)

    def run(self):
        self.sp.flushInput()

        while True:
            # look for incoming tornado request
            if not self.taskQ.empty():
                task = str(self.taskQ.get())

                # send it to the arduino
                self.sp.write(task + "\n");
  
            # look for incoming serial data
            if (self.sp.inWaiting() > 0):
                result = self.sp.readline().replace("\n", "")

                # send it back to tornado
                self.resultQ.put(result)

            time.sleep(0.05);
