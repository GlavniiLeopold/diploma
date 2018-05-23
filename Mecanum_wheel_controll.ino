#include "MeOrion.h"
//#include <Arduino.h>
#include "SoftwareSerial.h"
#include <Wire.h>

MeUSBHost usbhost(PORT_3);
MeJoystick joystick(PORT_6);

MeEncoderNew motor1(0x09, SLOT1); 
MeEncoderNew motor2(0x09, SLOT2); 
MeEncoderNew motor3(0x0A, SLOT1); 
MeEncoderNew motor4(0x0A, SLOT2); 

#define MAX 150
#define DEFAULTSPEED 100
#define MID    0
#define FRONT  200
#define BACK   -200
#define LEFT   -200
#define RIGHT  200
#define D2R PI / 180
#define K 12

typedef struct {
  signed int turn;
  signed int backfront;
  signed int leftright;
  unsigned char biaskey;
}RCDATA;
RCDATA Rcdata;

double a = 0;

int moveSpeed = DEFAULTSPEED;
unsigned char stop_flag=0;
unsigned char Rc_Flag = 0;

void parseJoystick(unsigned char * buf)
{
  
  //Rcdata.turn = buf[0] - MID;  //Remote control receive data
  Rcdata.leftright = buf[2];  //Remote control receive data
  Rcdata.backfront = buf[3];
  Rcdata.biaskey   = buf[4];
  /*Serial.print(Rcdata.turn);
  Serial.print('t');
  Serial.print(Rcdata.leftright);
  Serial.print('t');
  Serial.print(Rcdata.backfront);
  Serial.print('t');
  Serial.println(Rcdata.biaskey);
  delay(20);*/
}

void usbdataprocess()
{
    
  if(!usbhost.device_online)
  {
    usbhost.probeDevice();//Device is not online ,initialization 
    Serial.println("failed");//when you see the serial led blink quickly,it means the mcu doesn't receive the rc data.so,reset the mcu board and uplug the port of the rcceiver
    Rc_Flag = 1;
  }
  else
  {
    Rc_Flag = 0;
    int len = usbhost.host_recv();//Read data length
    parseJoystick(usbhost.RECV_BUFFER);
  }
}

boolean enc_A, enc_A_prev;
boolean enc_B;

double Enc_read()
{  
  enc_A = digitalRead(10);
  enc_B = digitalRead(11);

  if(!enc_A && enc_A_prev)
  {
    if (enc_B)   
      a += K;
    else
      a -= K; 
  }  
  else if(enc_A && !enc_A_prev)
  {
    if (enc_B)   
      a -= K;
    else
      a += K; 
  }
  enc_A_prev = enc_A;
  
  Serial.println(a);
}

void motorprocess()
{  
  int x = joystick.readX();
  int y = joystick.readY();
  
    
  if(y < 50 && y > -50)
  {
    if(x < 50 && x > -50)
    {
       //if(Rcdata.biaskey == 143) TurnLeft_run();  //Spot Turn To Left
       //else if(Rcdata.biaskey==47) TurnRight_run(); //Spot Turn To Right
       //else
       {
          stop_flag++;
          if(stop_flag>2)
          {
            Stop_run();    //Release the keys/Stop motor 
            stop_flag=0;
          } 
       }  
     }
     else if(x < LEFT)  Left_run();
     else if(x > RIGHT) Right_run();
   }
   else if(y > FRONT)
   {
      if(x < 50 && x > -50)      
        Forward_run();
      else if(x < LEFT)  
        LeftUp_run();
      else if(x > RIGHT) 
        RightUp_run();    
   }
   else if(y < BACK)
   {
     if(x < 50 && x > -50)        
       Backward_run();
     else if(x < LEFT)  
       LeftDown_run();
     else if(x > RIGHT) 
       RightDown_run();     
   }
}

void base_run( double alpha )
{  
  motor1.runSpeed(-MAX * cos((45 - alpha) * D2R));
  motor2.runSpeed(MAX * sin((45 - alpha) * D2R));
  motor3.runSpeed(MAX * cos((45 - alpha) * D2R));
  motor4.runSpeed(-MAX * sin((45 - alpha) * D2R)); 
}
 

void Forward_run()
{
  base_run(a); 
}

 void Backward_run()
{
  base_run(a + 180);
}

 void Right_run()
{
  base_run(a + 90);
}

 void Left_run()
{
  base_run(a - 90);
}
 
void LeftUp_run()
{
  base_run(a - 45);  
}

void LeftDown_run()
{
  double 
  base_run(a - 135); 
}

void RightUp_run()
{
  base_run(a + 45);
}

void RightDown_run()
{
  base_run(a + 135);
}

/*void TurnRight_run()
{
  motor1.runSpeed(-motor1speed,0);
  motor2.runSpeed(motor2speed,0);
  motor3.runSpeed(motor3speed,0);
  motor4.runSpeed(-motor4speed,0);
}

void TurnLeft_run()
{
  motor1.runSpeed(motor1speed,0);
  motor2.runSpeed(-motor2speed,0);
  motor3.runSpeed(-motor3speed,0);
  motor4.runSpeed(motor4speed,0);
}*/

void Speed_up()
{
  moveSpeed +=2;
  if(moveSpeed > MAX)  moveSpeed=MAX;
}
void Speed_down()
{
   moveSpeed -= 2;
  if(moveSpeed < 0)  moveSpeed=0;
}
void Stop_run()
{
  motor1.runSpeed(0);
  motor2.runSpeed(0);
  motor3.runSpeed(0);
  motor4.runSpeed(0);
}
 
void setup() {
  // put your setup code here, to run once:
  usbhost.init(USB1_0);//USB Remote Control Handle Initialization
  delay(10);
  motor1.begin();
  motor2.begin();
  motor3.begin();
  motor4.begin();
  delay(10);
  
  Serial.begin(9600);
  
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  
  motor1.setMode(0);  //0:I2C_MODE;1:PWM_MODE;2:PWM_I2C_PWM;
  motor2.setMode(0);
  motor3.setMode(0);  //0:I2C_MODE;1:PWM_MODE;2:PWM_I2C_PWM;
  motor4.setMode(0);
  
  Stop_run();
}

void loop() {
 // usbdataprocess(); 
  
  Enc_read();
  
  motorprocess();
}
