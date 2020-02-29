#include <Servo.h>
#include "DHT.h"

#define baseServoPin 10
#define camerasServoPin 9
#define laserPin 11
#define distancePin A1


////////////////////////////////////////////SERVO LIMITS///////////////////////////////////////
#define baseServoStartPosition 60       //max <50-130> but better <60-120>  90-start position
#define baseServoMINPosition 40 
#define baseServoMAXPosition 80 

#define camerasServoStartPosition 20    //max <60-120> but better <80-120>  100-start position
#define camerasServoMINPosition 5
#define camerasServoMAXPosition 35
///////////////////////////////////////////////////////////////////////////////////////////////

#define info_time 1000

Servo baseServo;
Servo camerasServo;
DHT dht;

byte data[100];
int data_len=0, czas;
bool send_radio,distance_flag=false;
long previousMillis = 0;

void setup() 
{
 initServos();
 pinMode(laserPin,OUTPUT);
 pinMode(distancePin,INPUT);
 Serial.begin(9600);
 digitalWrite(laserPin,HIGH);
 delay(500);
 digitalWrite(laserPin,LOW);

 dht.setup(2);
}

void loop() 
{
  long currentMillis=millis();
  
  data_len=0;
  
  if(Serial.available()){
  GetSerial();
  //data[1]=3;
  switch(data[1]){
    case 2:
      controlLaser();
      break;
    
    case 3:                                   //standard PWM steering instruction
      getServo();
      break;
    
    case 4:
      getDHT();
      break;
      
    case 5:
      if (data[4])
        distance_flag=true;
      else
        distance_flag=false;
        break;
    }
  }
   if(distance_flag){
    if(currentMillis-previousMillis>=info_time){
      byte distance_data[5]={0xFF,5,1,1,measureDistance()};
      data_len=5;
      send_info(distance_data);
      previousMillis=currentMillis;
      Serial.flush();
    }
   Serial.flush();
}
}

void initServos()
{
  baseServo.attach(baseServoPin);
  camerasServo.attach(camerasServoPin);

  baseServo.write(baseServoStartPosition);
  camerasServo.write(camerasServoStartPosition);

}

void controllServo(int servo,int pos)          //(b - base sevo / c - camera servo,position in angles)
{
  if(servo == 0)
  {
    if(pos > baseServoMAXPosition)  pos=baseServoMAXPosition;
    if(pos < baseServoMINPosition)  pos=baseServoMINPosition;
    //Serial.print(pos);
    baseServo.write(pos);
    //digitalWrite(laserPin,HIGH);
  }
  if(servo == 1)  
  {
    if(pos > camerasServoMAXPosition) pos=camerasServoMAXPosition;
    if(pos < camerasServoMINPosition) pos=camerasServoMINPosition;
    //Serial.print(pos);
    camerasServo.write(pos);
    //digitalWrite(laserPin,HIGH);
  }
}

int measureDistance()
{
int measure = analogRead(distancePinfg);
int distance = 9462/(measure- 16.92);
if(distance<0)distance=0;
if(distance>160)distance=160;
return distance;
}

void GetSerial(){ 
  int i=0,len;
  char znak;
     do
     {
      len=Serial.available();
      if(len!=0)
      if(len)
      {
        for(i=0;i<len;i++)
        {
          znak=Serial.read();
          data[data_len]=znak;
          data_len++;
        }
      }
      send_radio=true;
     }while(data_len<4 || (data_len-4) < data[3]);

     
}

void getServo(){
    int temp;
    int servo_base=data[4];
    int servo_cameras=data[5];

    temp=map(servo_cameras,0,255,5,35);
    servo_cameras=temp;
    controllServo(1,servo_cameras);
    
    temp=map(servo_base,0,255,40,80);
    servo_base=temp;
    controllServo(0,servo_base);
    
}

void controlLaser(){
  if(data[4])
    digitalWrite(laserPin,HIGH);
  else
    digitalWrite(laserPin,LOW);
}

void  getDHT(){
  delay(dht.getMinimumSamplingPeriod());
  
  int temperature = dht.getHumidity();
  int humidity = dht.getTemperature();
  data_len=6;

  byte  info_tab[6]={0xFF,4,1,2,temperature,humidity};
  send_info(info_tab);
}

void send_info(byte tab[]){
  for(int i=0; i<data_len; i++)
    Serial.print(((char)tab[i]));
  //Serial.flush();
}
