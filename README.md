# Universal indoors robot platform
The robot was built of 30x30 aluminum profiles. It has been equipped with two DC motors with 24V supply voltage. The power source was two batteries placed in a pull-out drawer made of plastic. The whole was enclosed with 3 elements cut from checker plate: rear, roof and bumper. The sides are made of plastic covered with foil (carbon pattern).

Other part of the robot was external raspberry Pi 3b+ wich served as mqtt broker.

Later camera module (2 DOF) was added controlled thru MQTT broker. Drive was implemented using servos.
![communication schematic](/images/camera_module.jpg)

## Construction
DC motors, connected to wheels via gears, were placed in back part of the robot. In the front of the robot swivel wheel was located. Access to electronics and batteries was made possible by flaps with a magnetic latching mechanism.


## Control
Robot control was develop thru two source possibilities:
1. via RC pilot
2. via Keyboard (thru mqtt broker)

As for the speed control motors where controlled in open loop, by applying voltage propotional to PWM signal from microcontroller.

## Communication
![communication schematic](/images/rob_camera.png)

Information is transfered in set of instructions and data along it.

Data frame: [0xff,instruction,adress,length of data, data]



## Electronics

### Radio-UART communication module
- RFM69HCW radio
- arduino nano

### MQTT broker
- raspberry pi 3b+
- radio-uart communication module

### Robot
- arduino nano
- radio-uart communication module
- motor driver controller
- 3x step-down converter (12V, 5V, 3.3V)
- RC reciever
- fuse and safety switch

### Camera Module
- arduino nano
- radio-uart communication module
- 2x servos
- 3x step-down(12V, 5V, 3.3V)

## Requierments

- mosquitto
- paho.mqtt
- low power lab library for rfm69HCW, ([git](https://github.com/LowPowerLab/RFM69))

