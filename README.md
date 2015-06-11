# Arduino Yún IoT Dashboard

A weekend project to expose the actors and sensors on the Arduino Yún to the Internet. The values of the sensors will be pushed real-time to any HTML5 compliant web browser by using WebSockets.

The ATmega microcontroller will use the serial port to communicate to the Atheros Linux microcomputer on the Yún board. The Atheros Linux microcomputer will run a webserver that will server some HTML5 pages that use WebSockets to exchange real-time information between the client's browser and the Yún board. 

This project can be a template for many other projects which need to connect the Arduino Yún to the Internet.

## Do it yourself

1. Build the circuit according the Fritzing sketch
2. Compile and upload the Arduino sketch (./ATmega32u4/dashboard/dashboard.ino)
3. Connect the Arduino Yún to your network 
  * WiFi: http://www.arduino.cc/en/Guide/ArduinoYun#toc14
4. Retrieve the IP address of the Arduino Yún
5. SSH into the Arduino 
  * http://www.arduino.cc/en/Tutorial/LinuxCLI
6. Download and install the tornado web server on the Atheros Linux microcomputer
  * http://www.tornadoweb.org/en/stable/
7. Deploy the content of the folder "./AtherosAR9331" to the root's home folder "/root"
8. Disable the bridge software on the Atheros
  * Comment out the line "ttyATH0::askfirst:/bin/ash --login" in /etc/inittab
9. Let the web server software automatically start when the Atheros boots
  * Add the line "python /root/server.py" to /etc/rc.local
10. Reboot the Atheros
11. In a browser navigate to http://<the ip address of your yun>:8080

## What you need

1. Arduino Yún
2. HC-SR04 Ultrasonic Distance Sensor
3. Grove Temperature Sensor
4. Grove Rotary Angle Sensor
5. Grove Light Sensor
6. Green Led + 1 kOhm Resistor
7. Red Led + 1 kOhm Resistor
8. Yellow Led + 1 kOhm Resistor
9. Grove 2x16 LCD Display (connected via i2c)
