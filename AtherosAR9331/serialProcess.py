import serial
import time
import multiprocessing

class SerialProcess(multiprocessing.Process):

    def __init__(self, taskQ, resultQ):
        multiprocessing.Process.__init__(self)
        self.taskQ = taskQ
        self.resultQ = resultQ
        self.sp = serial.Serial(port='/dev/ttyATH0', baudrate=9600, timeout=2)

    def close(self):
        self.sp.close()

    def sendData(self, data):
        print "sendData start..."
        self.sp.write(data)
        print "sendData done: " + data

    def run(self):
        self.sp.flushInput()

        while True:
            # look for incoming tornado request
            if not self.taskQ.empty():
                task = str(self.taskQ.get())

                # send it to the arduino
                self.sp.write(task + "\n");
                print "arduino received from tornado: " + task

            # look for incoming serial data
            if (self.sp.inWaiting() > 0):
                result = self.sp.readline().replace("\n", "")

                # send it back to tornado
                self.resultQ.put(result)

            time.sleep(0.05);
