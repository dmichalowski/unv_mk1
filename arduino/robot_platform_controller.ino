#include <Coordinates.h>
#include <LiquidCrystal_I2C.h>
#include"Timer.h"

//pilot channels
#define joyRX 2
#define joyRY 3
#define joyLY 10
#define joyLX 11
#define potL 12
#define potR 13

//motors channels
#define Left_Motor_F 9 
#define Left_Motor_B 4
#define LeftMotorPWM 5
#define Right_Motor_F 8
#define Right_Motor_B 7
#define RightMotorPWM 6


//chanels limit values
#define CHANNEL_MIN 1100
#define CHANNEL_MID 1500
#define CHANNEL_MAX 1900
#define CHANNEL_DEAD_HI 1525
#define CHANNEL_DEAD_LO 1475
#define SQRT2 1.41421356237
#define  PI 3.1415926535897932384626433832795

#define SWITCH 2 

//global variables
 int velLX=0, velLY=0, velRY=0, velRX=0,data_len=0; 
 int servo_1=0, servo_2=0;
 volatile int czas=0;
 static byte data[10];
 String mode;
 bool switch_mode;
 LiquidCrystal_I2C lcd(0x3F,2,16);
 Coordinates coordinates;


 
 struct LCD_INFO        //struct with allow change values on display
{
  int what_to_print;    //main parametr defines with information will be print on display
  char mode;            //A-automatic M-manual
  int velocity_PWM;
  float temperature;
  float humidity;
};


byte deg[8] = {        //custom char for printing degrees
  B11100,
  B10100,
  B11100,
  B00000,
  B00000,
  B00000,
  B00000,
};
LCD_INFO lcd_info;


void setup(){
//run setup functions

  timer_init();
  init_io();
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0,0);
}

void loop(){
//process steering operations according to mode

changeMode();                       //updatting info about steering mode (a-automatic, m-manual)
if(lcd_info.mode=='m')                //MANUAL
{
  getData();                          //taking variables from pilots
  controlMotors(velRX,velRY);         //send info about PWM to motors
  switch_mode=true;
  SendServo();
}
else if(lcd_info.mode=='a')           //AUTOMATIC
{
 if(Serial.available())
 {
  data_len=0;
  GetSerial();                        //if there is something on serial, data saves in ,,data'' tab
 }
 checkInstruction(velRX,velRY);       //according to instruction from message, function gets data from it
 if(switch_mode)
 {
  for(int i=0;i<10;i++)
  data[i]=0;
  velRY=0;
  velRX=0;
  switch_mode=false; 
 }
 controlMotors(velRY,velRX);                     //send info about PWM to motors
 data_len=0;
}
/*
if(czas==1)                           //timer interupt change ,,czas'' to 1 after every second and run this case which updates LCD screen
{
  Serial.println(getValue(joyLX));
  lcd.clear();
  LCD_print();
  czas=0;
}
*/

}

void getData(){
//receive data for motors joy-joystick X-direction Y-velocity   
//getValue() process data from joystick

  velLX=getValue(joyLX);  //left joy
  velLY=getValue(joyLY);
  velRX=getValue(joyRX);  //right joy
  velRY=getValue(joyRY);
}

void controlMotors (int &PWM_L, int &PWM_R){
//according to mode this function send PWM info to next motor function with coordinates conversion or without it

  if(lcd_info.mode=='m')
  {
  PWM_L=velRX;
  PWM_R=velRY; 
  processCoordinates(PWM_L,PWM_R);
  }
  setMotor(PWM_L,PWM_R); 
}

void setMotor(int PWM_L, int PWM_R){
//this function set polarization and write PWM to right pins

  int temp=0;
  if(lcd_info.mode=='m')
    {
    if(velRX==0&&velRY==0)
    {
      //Serial.println("Both motors velocity = 0");
      //analogWrite(LeftMotorPWM,0);
      //analogWrite(RightMotorPWM,0);
    }
    else
    {
        if((PWM_L+PWM_R)>0)                           //set reverse polarization for intuitive backwards driving
        {
          temp=PWM_L;
          PWM_L=PWM_R;
          PWM_R=temp;
        }
    //////////////////////////////////PAS POZIOMY
    //prevent rapid change of polarization in vertical left and right sectors
    
    if((PWM_L>0 && PWM_R<0)||(PWM_L<0 && PWM_R>0))    //search values from sektors in which variables have different signs
    {
        if((PWM_L+PWM_R)>-75 && (PWM_L+PWM_R)<75)     //search values from vertical line beetwen <-75,75>
        {
          PWM_L=map(velRY,-255,255,-100,100);         //map and set velocity of turning depends on where vertical joystick is
          PWM_R=map(velRY,-255,255,100,-100);
        }
    }
  }
} 
  //////////////////////////////////SILNIK LEWY
  //set polarize depends on PWM for left motor
      if(PWM_L>0)
      {
       digitalWrite(Left_Motor_F,LOW);
       digitalWrite(Left_Motor_B,HIGH);
       //Serial.print("LEFT motor direction: FORWARD ");
      }
      else
      {
        digitalWrite(Left_Motor_F,HIGH);
        digitalWrite(Left_Motor_B,LOW);
        //Serial.print("LEFT motor direction: BACKWARD ");
      }
  //////////////////////////////////SILNIK PRAWY
  //set polarize depends on PWM for right motor
     if(PWM_R>0)
     {
       digitalWrite(Right_Motor_F,LOW);
       digitalWrite(Right_Motor_B,HIGH);
       //Serial.print("RIGHT motor direction: FORWARD ");
     }
     else
     {
        digitalWrite(Right_Motor_F,HIGH);
        digitalWrite(Right_Motor_B,LOW);
     }    
       
  PWM_L=abs(PWM_L);                                 //take absolute value from PWMs 
  PWM_R=abs(PWM_R);
  
 // lcd_info.velocity_PWM=map(PWM,0,255,0,100);
 
  PWM_L=255-PWM_L;                                  //because of the H-bridge reverse logic we must change values
  PWM_R=255-PWM_R;
  analogWrite(LeftMotorPWM,PWM_L);                  //writing PWM to pins
  analogWrite(RightMotorPWM,PWM_R);
}

void processCoordinates(int &PWM_L, int &PWM_R){
//this function changes parameters from cartesian to polar to get maximum accurancy and efficiency
//its not possible to get maximum values on both motors without this conversion in manual mode
 
 float velocityRightMotor=0;
 float velocityLeftMotor=0;
 
       coordinates.fromCartesian(PWM_L, PWM_R);
       coordinates.fromPolar(coordinates.getR(),coordinates.getAngle()+(float)PI/(float)4);
       PWM_L=coordinates.getY()*SQRT2;
       PWM_R=-(coordinates.getX()*SQRT2);
       
       PWM_L=min(PWM_L, 255);
       PWM_R=min(PWM_R, 255);
       PWM_L=max(-255, PWM_L);
       PWM_R=max(-255, PWM_R);
}


int getValue(int data){
//read values from pilot and map to PWM values <-255,255>
 
  int value=pulseIn(data, HIGH, 50000), newValue=0;
  checkVariables(value);
  newValue=map(value,1100,1900,-255,255);
  return newValue;
}

void checkVariables(int &variable){
//filtration of transsmision errors and setting ,,dead zone''
  
    if((variable) <= CHANNEL_MIN)                 //low errors
        {
                (variable) = CHANNEL_MIN;
        }
        if ((variable) <= CHANNEL_DEAD_HI && (variable) >= CHANNEL_DEAD_LO )      //checking deadzone
        {
                (variable) = CHANNEL_MID;
        }
        if((variable)>= CHANNEL_MAX)              //high errors
        {
                (variable) = CHANNEL_MAX;
        }
}

void changeMode(){
//setting mode according to position of left joystick

int dane=0;

  dane=getValue(joyLY);
  if(dane>100)
    lcd_info.mode='m';
  else
    lcd_info.mode='a';
    
//changing LCD view

  if(velLX>220)
    lcd_info.what_to_print=1;
  else if(velLX<-50)
    lcd_info.what_to_print=0;
}

void init_io(){
//setting all pin modes and LCD

  pinMode(joyRX,INPUT);
  pinMode(joyRY,INPUT);
  pinMode(joyLY,INPUT);
  pinMode(joyLX,INPUT);
  pinMode(potL,INPUT);
  pinMode(potR,INPUT);
  pinMode(1,INPUT);
  pinMode(Left_Motor_F,OUTPUT);
  pinMode(Left_Motor_B,OUTPUT);
  pinMode(Right_Motor_F,OUTPUT); 
  pinMode(Right_Motor_B,OUTPUT);
  pinMode(LeftMotorPWM,OUTPUT);
  pinMode(RightMotorPWM,OUTPUT); 
  lcd_init();
  Serial.begin(9600);
}


void timer_init(){
//initalize right values in timer interrupt register

  cli();
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
}
ISR(TIMER1_COMPA_vect){
//timer1 interrupt with 1Hz frequency
czas=1;                                 //set ,,czas'' variable to 1 to update LCD view
}

void lcd_init(){
//setting LCD and write default configuration
   
   lcd.begin();                  // initialize the lcd 
   lcd.createChar(0,deg);        //create custom char
   lcd.backlight();
   pinMode(SWITCH, INPUT);       //switch configuraion
             
   lcd_info.what_to_print=0;
   lcd_info.mode='a';            //default lcd information configuration
   lcd_info.velocity_PWM=0;
   lcd_info.temperature=0.0;
   lcd_info.humidity=0.0;    
}


void LCD_print(){
  
 lcd.clear();
 
 if(lcd_info.what_to_print == 1)                             //IF SWITCH ON
 {  
      lcd.setCursor(0,0);
      lcd.print("mode:");                                   //MODE
      if( lcd_info.mode=='m'|| lcd_info.mode=='M')          //manual
      {
        lcd.print("     MANUAL");
      }
      else if( lcd_info.mode=='a'|| lcd_info.mode=='A')    //automatic
      {
        lcd.print("     AUTO");
      }
      lcd.setCursor(0,1);                                   //VELOCITY
      lcd.print("velocity: ");
      lcd_info.velocity_PWM=map(getValue(joyRY),-255,255,-100,100);
      if(lcd_info.velocity_PWM<0)
        lcd_info.velocity_PWM*=(-1);
      lcd.print((int)lcd_info.velocity_PWM);
      lcd.print("%");          
 }
 else                                                       //IF SWITCH OFF
 {
      lcd.setCursor(0,0);                                   //TEMPERATURE
      lcd.print("temp:     ");
      lcd.print(lcd_info.temperature);
      lcd.write(byte(0));
      lcd.print("C");
      lcd.setCursor(0,1);
      lcd.print("humidity: ");                              //HUMIDITY
      lcd.print(lcd_info.humidity);  
      lcd.print("%");
 } 
}

void checkInstruction(int &PWM_L, int &PWM_R){
//read data according to instruction byte

  if(data[1]==1)                                     //standard PWM steering instruction
  {
    //getting PWM for left motor with sign
    if((char)data[4]=='+')
    PWM_L=(int)data[5];
    else
    PWM_L=(int)data[5]*(-1);
    
    //getting PWM for right motor with sign
    if((char)data[6]=='+')
    PWM_R=(int)data[7];
    else
    PWM_R=(int)data[7]*(-1);
  }
}

void GetSerial(){ 
//this function get data packet from serial port

  int i=0,len;
  char znak;
     do
     {
      len=Serial.available();                         //check how many variables is on serial port
        if(len)                                      
        {
          for(i=0;i<len;i++)
          {
            znak=Serial.read();                       //getting value from serial port
            data[data_len]=znak;                      //writing readed values to data tab
            data_len++;                               //increments variable which tells how many variables is in tab
          }
        }
     }while(data_len<4 || (data_len-4) < data[3]);    //loop which reads values until message ends
}

void SendServo(){
  int newValue,temp,switch_1;
  bool change=false;

  temp=getValue(potL);
  newValue=map(temp,-255,255,0,255);
  if(abs(newValue-servo_1)>2){
    servo_1=newValue;
    change=true;
  }
    
  temp=getValue(potR);
  newValue=map(temp,-255,255,0,255);
  if(abs(newValue-servo_2)>2){
    servo_2=newValue;
    change=true;
  }

 
  
  byte data_servo[6]={0xFF,3,3,2,servo_1,servo_2};


  if(change){
    for (int i=0; i<6; i++)
      Serial.print((char)data_servo[i]);
      //Serial.println();
    change=false;
  }
  if(lcd_info.what_to_print)
  lcd_info.what_to_print=0;
}
