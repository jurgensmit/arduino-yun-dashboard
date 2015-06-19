#!/usr/bin/python27
#
# Web server to serve the static pages located in the subfolder ./public
#  
# The server also handles WebSocket requests of clients to push and
# receive sensor values to the ATmega microcontroller in real-time.
#
import tornado.httpserver
import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.gen
import json
from tornado.options import define, options

import time
import multiprocessing
import serialProcess

# The default port to use for the server
define("port", default=8080, help="run on the given port", type=int)

# The WebSocket clients that are attached to this service
clients = []

# The latest data we have received from the arduino
latestData = {}

################################################################
# Arduino message handling
################################################################
def ProcessArduinoMessage(message):
    if message.find(":") == 1:
        command = message[0]
        parameters = message[2:]
        if command == "A":
            ProcessAngle(parameters)
        elif command == "L":   
            ProcessLedStatus(parameters)
        elif command == "D":   
            ProcessDistance(parameters)
        elif command == "T":   
            ProcessTemperature(parameters)
        elif command == "P":   
            ProcessLight(parameters)

def ProcessDistance(distance):
    latestData["distance"] = int(distance)
    SendLatestDataToClients()
    
def ProcessTemperature(temperature):
    latestData["temperature"] = int(temperature)
    SendLatestDataToClients()
    
def ProcessLight(light):
    latestData["light"] = int(light)
    SendLatestDataToClients()
    
def ProcessAngle(angle):
    latestData["angle"] = int(angle)
    SendLatestDataToClients()
    
def SendLatestDataToClients():
    for c in clients:
        c.write_message(json.dumps(latestData))

def ProcessLedStatus(parameters):
    if len(parameters) == 2:
        ledIndex = int(parameters[0])
        ledStatus = int(parameters[1])

        if ledStatus == 0:
            ledStatusAsString = "Off"
        else:
            ledStatusAsString = "On"

        if ledIndex == 0:
            latestData["yellowLedState"] = ledStatusAsString
        elif ledIndex == 1:
            latestData["greenLedState"] = ledStatusAsString
        elif ledIndex == 2:
            latestData["redLedState"] = ledStatusAsString
        
        SendLatestDataToClients()

################################################################
# Main Page Handler
################################################################
class IndexHandler(tornado.web.RequestHandler):
    def get(self):
        self.render("public/index.html")

################################################################
# WebSocket Handler
################################################################
class WebSocketHandler(tornado.websocket.WebSocketHandler):
    def open(self):
        print "new connection"
        clients.append(self)
        self.write_message(json.dumps(latestData))

    def on_message(self, message):
        self.process_client_message(str(message))

    def on_close(self):
        print "connection closed"
        clients.remove(self)

    def process_client_message(self, message):
        arduino_message = ""
        if message.find("toggle_led:") == 0:
           
            led_color = message.replace("toggle_led:", "")
            led_index = -1
            if led_color == "yellow":
                led_index = 0
            elif led_color == "green":
                led_index = 1
            elif led_color == "red":
                led_index = 2
           
            if led_index >= 0:
                arduino_message = "L:" + str(led_index) + "@"
        
        elif message.find("get_all_data:") == 0:
            arduino_message = "@X:@"
        
        if(arduino_message <> ""):
            q = self.application.settings.get("queue")
            q.put(arduino_message)

################################################################
# Main
################################################################
def main():

    taskQ = multiprocessing.Queue()
    resultQ = multiprocessing.Queue()

    sp = serialProcess.SerialProcess(taskQ, resultQ)
    sp.daemon = True
    sp.start()

    # Wait a second before sending the first request to get
    # the current arduino values
    time.sleep(1)
    taskQ.put("@X:@")

    tornado.options.parse_command_line()

    app = tornado.web.Application(
        handlers = [
            (r"/ws", WebSocketHandler),
            (r"/", IndexHandler),
            (r"/(.*)", tornado.web.StaticFileHandler, { "path": "/root/public" })
        ],
        queue = taskQ
    )
    httpServer = tornado.httpserver.HTTPServer(app)
    httpServer.listen(options.port)
    print "Listening on port:", options.port

    def checkResults():
        if not resultQ.empty():
            message = resultQ.get()
            ProcessArduinoMessage(str(message.replace("\n", "").replace("\r", "")))

    mainLoop = tornado.ioloop.IOLoop.instance()
    scheduler = tornado.ioloop.PeriodicCallback(checkResults, 10, io_loop = mainLoop)
    scheduler.start()
    mainLoop.start()

if __name__ == "__main__":
    main()



