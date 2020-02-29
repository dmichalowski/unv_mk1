#!/usr/bin/env python3

import paho.mqtt.client as mqtt
from time import sleep
import serial

#after connecting to mosquito: (rc - error code, 0 = good)
def on_connect(client, userdata, flags, rc):
  print("Connection established " + str(rc))
  client.subscribe("topic/vel")

#after something gets published to subscribed topic
def on_message(client, userdata, msg):
    #printing acquired parameters for debug
    print("New parameters acquired: [" + chr(msg.payload[1]) + str(msg.payload[2])
        + "," + chr(msg.payload[3]) + str(msg.payload[4]) + "]")
    #building msg to ardunio [0xff,instruction,adress,length of data, data[]]
    serialMsg = bytes([255,1,msg.payload[0],
                       len(msg.payload)-1]) + msg.payload[1:len(msg.payload)]
    print(serialMsg)
    with serial.Serial('/dev/ttyUSB0',9600,timeout=None) as arduinoCommunication:
      arduinoCommunication.write(serialMsg)
      arduinoCommunication.flush()
    
client = mqtt.Client()
client.connect("localhost",1883,60)

client.on_connect = on_connect
client.on_message = on_message

#after that reacting only on callbacks
client.loop_forever()


