#include <RFM69.h>
#include <SPI.h>

#define NETWORKID     0
#define MYNODEID      3 

#define FREQUENCY   RF69_433MHZ

RFM69 radio;

static byte data[10];
int data_len;
byte TONODEID;
bool send_radio;

void setup()
{
  // Open a serial port so we can send keystrokes to the module:
  Serial.begin(9600);
 // Serial.print("Node ");
//  Serial.print(MYNODEID,DEC);
//  Serial.println(" ready");  

  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower();
}

void loop()
{
  if(radio.receiveDone())
    GetRF();
  data_len=0;
  if(Serial.available())
    SendRF();
  data_len=0;
}


void GetRF(){
  data_len=0;
    for(int i=0; i<radio.DATALEN ; i++)
    {
      data[i]=radio.DATA[i];
      //Serial.println((char)data[i]);
      data_len++;
    }
    SendSerial();
}

void SendSerial(){
  if(data[0]==0xFF){
    for(int i=0; i<data_len;i++)
    {
     Serial.print((char)data[i]);
    }
    Serial.flush();
    data_len=0;
  }
}

void SendRF(){
   GetSerial();
   if(send_radio){
    TONODEID=data[2];
    radio.send(TONODEID, data, data_len);
    data_len=0;
    send_radio=false;
   }
}

void GetSerial(){ 
  int i=0,len;
  char znak;
     do
     {
      len=Serial.available();
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
   Serial.flush();
}
