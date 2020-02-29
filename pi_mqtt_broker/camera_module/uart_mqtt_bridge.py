#!/usr/bin/env python3

import paho.mqtt.client as mqtt
from time import sleep
#import serial
import logging
from uart import UartCommunication
from queue import Queue

#global uart
uart = UartCommunication('/dev/ttyUSB0')
dataQueue = Queue()
debug = True

def on_message(client, userdata, msg):
    global dataQueue
    #global dataQueue
    #printing acquired parameters for debug
    if debug:
        print("New parameters acquired: [" + chr(msg.payload[1]) + str(msg.payload[2])
            + "," + chr(msg.payload[3]) + str(msg.payload[4]) + "]")
    #building msg to ardunio [0xff,instruction,adress,length of data, data[]]
    serialMsg = bytes([255,1,msg.payload[0],
                       len(msg.payload)-1]) + msg.payload[1:len(msg.payload)]
    #uart.writePackage(serialMsg)
    #print("przed")
    dataQueue.put(serialMsg)

#after something gets published to subscribed topic
def on_message_dht11(client, userdata, msg):
    if debug:
        print("Got from MQTT topic [DHT11]", msg.payload)
    serialMsg = bytes([255, 4, msg.payload[0], len(msg.payload)-1]) + msg.payload[1:len(msg.payload)]
    dataQueue.put(serialMsg)

def on_message_laser(client,userdata, msg):
    global dataQueue
    if debug:
        print("Got from MQTT topic [LASER]", msg.payload)
    serialMsg = bytes([255, 2, msg.payload[0], len(msg.payload)-1]) + msg.payload[1:len(msg.payload)]
    dataQueue.put(serialMsg)

def on_message_servo(client,userdata, msg):
    if debug:
        print("Got from MQTT topic [SERVO]", msg.payload)
    serialMsg = bytes([255, 3, msg.payload[0], len(msg.payload)-1]) + msg.payload[1:len(msg.payload)]
    dataQueue.put(serialMsg)

def on_message_sharp(client,userdata, msg):
    if debug:
        print("Got from MQTT topic [SHARP]", msg.payload)
    serialMsg = bytes([255, 5, msg.payload[0], len(msg.payload)-1]) + msg.payload[1:len(msg.payload)]
    dataQueue.put(serialMsg)
    
def on_disconnect(client, userdata,rc = 0):
    if debug:
        print("DisConnected result code " + str(rc))
    client.loop_stop()

def startClients(nclients):
    clients=[]
    mqtt.Client.connected_flag=False
    #create clients
    for i in range(nclients):
        cname = "Client"+str(i)
        client = mqtt.Client(cname)
        clients.append(client)
    for client in clients:
        client.connect("localhost",1883,60)
        client.loop_start()
    return clients

# def setCameraAxis(x,y,msg):
#     serialMsg = bytes([255,msg["instruction"],msg["adress"],msg["data_length"]]
#          + msg["data"])

def initClients():
    clients = startClients(5)

    clients[0].on_message = on_message
    clients[0].on_disconnect = on_disconnect

    clients[1].subscribe("topic/request/dht11")
    clients[1].on_message = on_message_dht11
    clients[1].on_disconnect = on_disconnect

    clients[2].subscribe("topic/dev/laser")
    clients[2].on_message = on_message_laser
    clients[2].on_disconnect = on_disconnect
    
    clients[3].subscribe("topic/dev/servo")
    clients[3].on_message = on_message_servo
    clients[3].on_disconnect = on_disconnect

    clients[4].subscribe("topic/request/sharp")
    clients[4].on_message = on_message_sharp
    clients[4].on_disconnect = on_disconnect
    return clients

def handleMsg(msg, client):
    if msg == None:
        return 
    if(msg[0] == 4):
        if(msg[2] == 2):
            if debug:
                print("[GOING TO MQTT] topic [DHT11]: ",msg[3:len(msg)])
            client.publish("topic/dev/dht11", msg[3:len(msg)])
    elif(msg[0] == 5):
        if(msg[2] == 1):
            if debug:
                print("[GOING TO MQTT] topic [SHARP]: ",msg[3:len(msg)])
            client.publish("topic/dev/sharp", msg[3:len(msg)])

def main():
    global dataQueue
    #uart = UartCommunication('/dev/ttyUSB0')
    clients = initClients()

    while(True):
        #with uart.lock:
        #1-----------
        if uart.readoutSerial.inWaiting():
            data = uart.readPackage()
            if debug:
                print("[DATA RECEIVED]: ", data)
            handleMsg(data, clients[0])
        #--------------
            #uart.writePackage(dataQueue.get())
        #sleep(0.5)
        if not dataQueue.empty():
            queueItem = dataQueue.get()
            if debug:
            #print("[INSTRUCTION]", queueItem[1])
                print("[GOING TO SERIAL]", queueItem)
            #print("flag 1")
            #uart.writePackage(dataQueue.get())
            uart.sendSerial.write(queueItem)
            #sleep(0.5)
            #print("flag 2")
            

if __name__ == '__main__':
    print("go")
    main()