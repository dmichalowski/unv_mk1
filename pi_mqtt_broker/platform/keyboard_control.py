import paho.mqtt.client as mqtt
from time import sleep
import pygame
import serial


client = mqtt.Client()
client.connect("localhost",1883,60)


pygame.init()

window = pygame.display.set_mode((800,600))
controlTable = []
upFlag,downFlag,rightFlag,leftFlag,speedUpFlag,speedDownFlag = False,False,False,False,False,False
speed,acceleration = 0,0
left_dir,right_dir = '+','+'
left_vel,right_vel = 0,0
pygame.display.set_caption("Window")
clock = pygame.time.Clock()
gameLoop=True

while gameLoop:
 
    for event in pygame.event.get():
 
        if (event.type==pygame.QUIT):
             gameLoop=False
             
        if (event.type==pygame.KEYDOWN):
            if (event.key==pygame.K_LEFT):
                leftFlag = True
                #print('W lewo')
                
            if (event.key==pygame.K_RIGHT):
                rightFlag = True
                #print('W prawo')
 
            if (event.key==pygame.K_UP):
                upFlag = True
                #print('Do przodu')
 
            if (event.key==pygame.K_DOWN):
                downFlag = True
                #print('W tyl')
                
            if (event.key==pygame.K_w):
                speedUpFlag = True
                acceleration = 5
                #print('przyspiesz')
                
            if (event.key==pygame.K_s):
                speedUpFlag = True
                acceleration = -5
                #print('zwolnij')
                
        if (event.type==pygame.KEYUP):
            if (event.key==pygame.K_LEFT):
                leftFlag = False
 
            if (event.key==pygame.K_RIGHT):
                rightFlag = False
 
            if (event.key==pygame.K_UP):
                upFlag = False
 
            if (event.key==pygame.K_DOWN):
                downFlag = False
                
            if (event.key==pygame.K_w):
                acceleration = 0
                
            if (event.key==pygame.K_s):
                acceleration = 0
    
    window.fill((0,0,0))
    clock.tick(10)
    #print("dupeczka")
    
    speed += acceleration
    if speed < 0: speed = 0
    if speed > 255: speed = 255
        
    
    if not(upFlag and downFlag and rightFlag and leftFlag):
        left_vel, left_dir = 0, '+'
        right_vel, right_dir = 0, '+'
    
    #forward and backward
    if upFlag:
        left_vel, left_dir = speed, '+'
        right_vel, right_dir = speed, '+'
    elif downFlag:
        left_vel, left_dir = speed, '-'
        right_vel, right_dir = speed, '-'
        
    #left and right
    if rightFlag:
        left_vel, left_dir = speed, '+'
        right_vel, right_dir = speed, '-'
    elif leftFlag:
        left_vel, left_dir = speed, '-'
        right_vel, right_dir = speed, '+'
    
    #backward right & left
    if rightFlag and downFlag:
        left_vel, left_dir = speed, '-'
        right_vel, right_dir = int(0.5 * speed), '-'
    elif leftFlag and downFlag:
        left_vel, left_dir = int(0.5 * speed), '-'
        right_vel, right_dir = speed, '-'
    
    #forward right & left   
    if rightFlag and upFlag:
        left_vel, left_dir = speed, '+'
        right_vel, right_dir = int(0.5 * speed), '+'
    elif leftFlag and upFlag:
        left_vel, left_dir = int(0.5 * speed), '+'
        right_vel, right_dir = speed, '+'
        
    controlTable = bytes([2,ord(left_dir),left_vel,ord(right_dir),right_vel])
    #print(controlTable)
    
    client.publish("topic/vel", controlTable)
    pygame.display.flip()
    
    
 
pygame.quit()
