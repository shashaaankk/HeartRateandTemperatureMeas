#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <TimerOne.h> 
#include <GSM.h>  

const int redLED = 3;              //Indiacates Elivated Heart Rate
const int yelLED = 13;             //Indicated Elevated Body Temparature 

const int PulseWire = 4;   //Heart Rate input port
const int tempWire = A0;   //Body Temperature input port

int HBStart = 2;                //Press Button @ Digital Pin 2 to start Hearbeat measurement 
int HBStartCheck = 0;           //Flag indicating the begining of measurement 
int HBCheck = 0;                //Flag inidicating if check has been requested or not 

int HBperMin = 0;               //Initial value of BPM
int HBCount = 0;                // Beat Counter

int TimeinSec = 0;              //Timer 

float normalTemp = 37;          //Normal Values
float bodyTempMeas = 0;

LiquidCrystal_I2C lcd(0x20,16,2);   //LCD Object
SoftwareSerial sim8001(0,1);

void setup() {
  
 pinMode(PulseWire, INPUT);
 pinMode(HBStart, INPUT_PULLUP);

 pinMode(redLED,OUTPUT);
 pinMode(yelLED,OUTPUT);

 Timer1.initialize(800000); 
 Timer1.attachInterrupt( timerIsr );

 lcd.begin(16,2);
 sim8001.begin(9600);
 Serial.begin(9600);
}

void loop() {
 
  //Body Temparature
  bodyTempMeas = getTemp(tempWire);

  //Heart Rate and Printing
  if(digitalRead(HBStart) == LOW)
  {
     HBStartCheck = 1;   
  }
  if(HBStartCheck == 1)
  { 
    lcd.backlight(); 
    Serial.println("Calculating...");
    
    if((digitalRead(PulseWire) == HIGH) && (HBCheck == 0))
    {
      HBCount = HBCount + 1;
      HBCheck = 1;  
      
    }
           
    if((digitalRead(PulseWire) == LOW) && (HBCheck == 1))
    {
      HBCheck = 0;   
    }

    if(TimeinSec == 5)
    {
      HBperMin = HBCount * 12;
      HBStartCheck = 0; 
      HBCount = 0;
      TimeinSec = 0; 
      
      lcd.clear();
      lcd.print("Temperature: ");
      lcd.setCursor(13, 0);   //Column, Row
      lcd.print(bodyTempMeas); 
      lcd.setCursor(2, 1);    
      lcd.print("Heart BPM: ");
      lcd.setCursor(13, 1);   
      lcd.print(HBperMin); 

      if (HBperMin > 90)
      {
          blink(redLED);
          SendSMS_BPM();
          if(sim8001.available())  {
            Serial.write(sim8001.read());   
          }                    
      }
        
      //2
      if (bodyTempMeas > normalTemp)
      {
        blink(yelLED);
        SendSMS_Temp();
        if(sim8001.available())  {
        Serial.write(sim8001.read());   
        }
      } 
      
 
     } 
  }    
}

void timerIsr()
{
  if(HBStartCheck == 1)
  {
    TimeinSec = TimeinSec + 1;
  }
}

// LED Blinking
void blink(int pin)
{ 
  for (int i=0; i<2; i++)
  {
    digitalWrite(pin,HIGH);
    delay(100);
    digitalWrite(pin,LOW);
    delay(100); 
  }
}

// Body Temperature Measurement
float getTemp(int tempWire)
{ 

  float val = analogRead(tempWire);
  //Serial.println(tempWire);  
  float bodyTemp = (val*500)/1023;
  return  bodyTemp;
}

//Sending Alert
void SendSMS_Temp()
{
  Serial.println("Sending sms...");
  sim8001.print("AT+CMGF=1");                      //Setting to SMS Mode
  delay(100);
  sim8001.print("AT+CMGS=\"+919036751497\"\r");      //Get Number
  delay(100);
  sim8001.println("YOU HAVE HIGH TEMPARATURE\r");        //SMS          <---  THE LINE
  delay(100);
  sim8001.print((char)26);                           //Datasheet Requirement
  Serial.println("Text Sent : YOU HAVE HIGH TEMPARATURE");
  delay(100);
}

void SendSMS_BPM()
{
  Serial.println("Sending sms...");
  sim8001.print("AT+CMGF=1\r");                      
  delay(100);
  sim8001.print("AT+CMGS=\"+919036751497\"\r");
  delay(100);
  sim8001.print("YOUR HEART IS UNDER STRESS, CALM DOWN!\r");
  delay(100);
  sim8001.print((char)26);                           //Datasheet Requirement
  sim8001.println();
  Serial.println("Text Sent : YOUR HEART IS UNDER STRESS, CALM DOWN! ");
  delay(100);
}



