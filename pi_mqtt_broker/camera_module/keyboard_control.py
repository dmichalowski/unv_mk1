import paho.mqtt.client as mqtt
from time import sleep
import pygame
import serial


client = mqtt.Client()
client_servo = mqtt.Client()
client.connect("localhost",1883,60)
client_servo.connect("localhost",1883,60)

client_dht11 = mqtt.Client()
client_dht11.connect("localhost",1883,60)

client_sharp = mqtt.Client()
client_sharp.connect("localhost",1883,60)

client_dht11.subscribe("topic/dev/dht11")
client_sharp.subscribe("topic/dev/sharp")

temp,hum,dist = 0,0,0

def listText(texts):
    textRects = []
    centerY = 0
    for t in texts:
        rect = t.get_rect()

        centerY += rect.height // 2
        w = rect.width
        rect.center = (w//2, 600 - centerY) 

        centerY += rect.height // 2

        textRects.append(rect)
    return textRects

def displayTextBlocks(displaySurface, texts, textRects):
    for t, r in zip(texts,textRects):
        displaySurface.blit(t, r)
    return displaySurface


def checkVariables(speed):
    if speed < 0: speed = 0
    if speed > 255: speed = 255
    return speed

def on_message_dht11 (client,userdata,msg):
    global temp,hum
    temp,hum = msg.payload[0],msg.payload[1]
    
def on_message_sharp (client,userdata,msg):
    global dist
    print("[SHARP]", msg.payload)
    dist = msg.payload[0]

def on_disconnect (client, userdata,rc = 0):
    client.loop_stop()
    
client_dht11.on_message = on_message_dht11
client_sharp.on_message = on_message_sharp
    
pygame.init()

#styles
white = (255, 255, 255) 
green = (0, 255, 0) 
blue = (0, 0, 128)
font = pygame.font.Font('freesansbold.ttf', 16)

#costam
window = pygame.display.set_mode((800,600))
controlTable = []
upFlag,downFlag,rightFlag,leftFlag,laserFlag,tempFlag,distanceFlag= False,False,False,False,True,True,True
speed_1,speed_2,acceleration_1,acceleration_2 = 0,0,0,0
camera_vel,base_vel=0,0
publish_time, old_publish_time = 0,50
laserState, distanceState = 0,0
tempRequest = 0
pygame.display.set_caption("Window")
clock = pygame.time.Clock()
gameLoop=True

client_dht11.loop_start()
client_sharp.loop_start()
    
while gameLoop:
 
    for event in pygame.event.get():
 
        if (event.type==pygame.QUIT):
             gameLoop=False
             
        if (event.type==pygame.KEYDOWN):
            if (event.key==pygame.K_LEFT):
                leftFlag = True
                acceleration_1 = -5
                #print('W lewo')
                
            if (event.key==pygame.K_RIGHT):
                rightFlag = True
                acceleration_1 = 5
                #print('W prawo')
 
            if (event.key==pygame.K_UP):
                upFlag = True
                acceleration_2 = 5
                #print('Do przodu')
 
            if (event.key==pygame.K_DOWN):
                downFlag = True
                acceleration_2 = -5
                #print('W tyl')
            if (event.key==pygame.K_l):
                laserFlag=True
                if (laserState==0):
                    laserState = 1
                elif (laserState==1):
                    laserState=0
            if (event.key==pygame.K_t):
                tempFlag=True
                tempRequest=1
            if (event.key==pygame.K_d):
                distanceFlag=True
                if (distanceState==0):
                    distanceState = 1
                elif (distanceState==1):
                    distanceState=0
                
        if (event.type==pygame.KEYUP):
            if (event.key==pygame.K_LEFT):
                leftFlag = False
                acceleration_1 = 0
            if (event.key==pygame.K_RIGHT):
                rightFlag = False
                acceleration_1 = 0
            if (event.key==pygame.K_UP):
                upFlag = False
                acceleration_2 = 0
            if (event.key==pygame.K_DOWN):
                downFlag = False
                acceleration_2 = 0
            if(event.key==pygame.K_l):
                laserFlag=False
            if(event.key==pygame.K_t):
                tempFlag=False
            if(event.key==pygame.K_d):
                distanceFlag=False
                
    window.fill((0,0,0))
    clock.tick(100)
    #print("dupeczka")
    
    speed_1 += acceleration_1
    speed_2 += acceleration_2
    
    speed_1 = checkVariables(speed_1)
    speed_2 = checkVariables(speed_2)
    
        
    #forward and backward
    if upFlag or downFlag:
        camera_vel=speed_2
        #print("Speed 2 =",camera_vel)
    #left and right
    if rightFlag or leftFlag:
        base_vel=speed_1
        #print("Speed_1 =",base_vel)
        
    controlTable = bytes([3,camera_vel,base_vel])
    #print(controlTable)
    publish_time = pygame.time.get_ticks()
    if (publish_time >= old_publish_time):
        #print("publish!")
        old_publish_time=publish_time+50
        client_servo.publish("topic/dev/servo", controlTable)
        
    if (laserFlag==False):
        #print("Laser: ",laserState)
        controlTable = bytes([3,laserState])
        client.publish("topic/dev/laser",controlTable)
        laserFlag=True
        
    if (tempFlag==False):
        #print("Temp Request!!! ",tempRequest)
        controlTable = bytes([3,tempRequest])
        client.publish("topic/request/dht11",controlTable)
        tempFlag=True
        tempRequest=0
        
    if (distanceFlag==False):
        #print("Change distance state: ",distanceState)
        controlTable = bytes([3,distanceState])
        client.publish("topic/request/sharp",controlTable)
        distanceFlag=True


    #-------------------text display---------------------
    
    
    
    temperature = font.render('Temperature: ' + str(temp) + " deg", True, green, blue)
    humidity = font.render('Humidity: ' + str(hum) + " %", True, green, blue) 
    distance = font.render('Distance: ' + str(dist) + " cm", True, green, blue)

    texts = ([temperature,humidity,distance])
    textRects = listText(texts)

    window = displayTextBlocks(window, texts, textRects)

    
    
        
    pygame.display.flip()
        
 
pygame.quit()
  
