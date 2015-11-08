    // Garage 2x Door Monitor and Fingerprint Opener
// Dave Astolfo www.plastibots.com
// Uses Blynk to process and display info over net from IOS/Android
// Uses Adafruit CC3000 breakout board for WiFi comms
// Uses Optiboot Nano bootloader (use Arduino v1.0.5 me)
// Uses watchdog timer due to issue with either Nano or CC3000 crashing - forces restart

// ********  Must select board:  [Optiboot] Arduino Duemilaove or Nano w/Atmega328 **********

#include <avr/wdt.h>         // required for watchdog timer
#include <SoftwareSerial.h>
#include "FPS_GT511C3.h" //the fps (fingerprint scanner) library
//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

// These are the interrupt and control pins for 3000
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

#include <SPI.h>
#include <Adafruit_CC3000.h>
#include <BlynkSimpleCC3000.h>


FPS_GT511C3 fps(2, 4); //RX, TX

const int door1ClosePin = 6;
const int door2ClosePin = 7;
const int door1LimitPin = A0;
const int door2LimitPin = A1;
const int OKLedPin = A5;
const int NotOKLedPin = A4;
const int doorStatLEDPin = 8; 
const int delayVal = 100; //in milliseconds
int i = 0;
int doorState = 0;  //value
int d1S, d2S, d1LimVal, d2LimVal;
boolean door_p1, door_p2, door_p3, door_p4;  //bits for door 1 pwd
unsigned long pwdEntryStartTime = 0;
long interval = 30000;  //30 second grace period to retain password to allow opening of doors.

// You should get Auth Token in the Blynk App. Go to the Project Settings (nut icon).
char auth[] = "authcodehere";


void setup()
{

  //Serial.begin(9600);  //for debugging - disable once live.
  
  
  pinMode(OKLedPin, OUTPUT);
  pinMode(NotOKLedPin, OUTPUT);  
  pinMode(doorStatLEDPin, OUTPUT);    
  pinMode(door1ClosePin, OUTPUT);        
  pinMode(door2ClosePin, OUTPUT);        
  pinMode(door1LimitPin, INPUT);    
  pinMode(door2LimitPin, INPUT);      

  
  fps.Open();
  fps.SetLED(true); //the fps LED
  fps.UseSerialDebug = false; //set to true for fps debugging through serial
  blink(OKLedPin, 1000, 1);
  Blynk.begin(auth, "SSID", "PASSCODE", WLAN_SEC_WPA2);
  blink(OKLedPin, 500, 3);
  wdt_enable(WDTO_8S);   //turns on watchdog timer with 8 second countdoor.  Sketch requires wtd_reset() to reset this, else a reboot is forced
}

void loop()
{

  wdt_reset();  //reset as it goes through each loop - necessary to ensure the watchdog timer does not reset the Duino when it's working fine.
  
  if (!cc3000.checkConnected())
  {
    wdt_reset(); //reset the watchdog timer in any normal op that may take time.
    Blynk.begin(auth, "SSID", "PASSCODE", WLAN_SEC_WPA2);
    blink(OKLedPin, 500, 3);
  }

  Blynk.run();                //start polling the system and reporting to the Blynk app.
  Blynk.virtualWrite(4, 1);   //sends a heartbeat to the Blynk app.
  
  // Only want to send / check door state values every 0.5s (500ms) seconds.  Based on the delay value set in the main loop
  // this takes care of it without using a timer - this also does not slow down the main loop
  i++;
  if (i == (1000 / delayVal) - 1)
  {
   Blynk.virtualWrite(4, 0); 
   checkDoorState();
   i = 0;
   blink(OKLedPin, 100, 1);  
  }
  //end timing a door state check
  
  
  //scan and identify the finger when one is put on it
  chkForFingerPrint();

  delay(delayVal);  
  
}



//************************************************************************************************************************************************
//future enhancement - allow for enrolling new fingerprints
void Enroll()
{
}


//************************************************************************************************************************************************
void chkForFingerPrint()
{
	if (fps.IsPressFinger())
	{
                wdt_reset();	//reset the watchdog timer for any normal op that may take time.	
                fps.CaptureFinger(false);
		int id = fps.Identify1_N();
		if (id < 200)
		{
			//Serial.print("Verified ID:");
			//Serial.println(id);
                        blink(OKLedPin, 100, 2);
                        Action(1);   //Open the Garage door
		}
		else
		{
			//Serial.println("Finger not found");
                        blink(NotOKLedPin, 100, 3);
		}
	}  
}

//************************************************************************************************************************************************
void blink(int whichLED, int dly, int numTimes)
{
  for (int i = 1; i<=numTimes; i++){  
    digitalWrite(whichLED, HIGH);
    delay(dly);
    digitalWrite(whichLED, LOW);
    delay(100);    
  }
}

//************************************************************************************************************************************************
void checkDoorState()
{
  //the master controller will send values to this analog pin when the doors are open. When closed, the value will be 0
  //will need to have 4 door state values.
  // door 1 = open, door 1 & 2 open, door 2 open.  all doors closed
  d1LimVal = analogRead(door1LimitPin);
  d2LimVal = analogRead(door2LimitPin);
  
  //Serial.print("Limit1Val:  ");
  //Serial.print(d1LimVal);
  //Serial.print("   Limit2Val:  ");  
  //Serial.println(d2LimVal);
    
    
  if ((d1LimVal >= 1000) && (d2LimVal >= 1000))  //both door limit switches are closed. 1023
  {
    d1S = 1;
    d2S = 1;
    blink(NotOKLedPin, 100, 1);
    msgOut(doorStatLEDPin, 1);
  }
  else if ((d1LimVal >= 1000) && (d2LimVal < 50))
  {
    d1S = 1;
    d2S = 0;
    blink(NotOKLedPin, 100, 1);
    msgOut(doorStatLEDPin, 1);   
  }
  else if ((d1LimVal < 50) && (d2LimVal >= 1000))
  {
    d1S = 0;
    d2S = 1;
    blink(NotOKLedPin, 100, 1);
    msgOut(doorStatLEDPin, 1);    
  }
  else
  {
    d1S = 0;
    d2S = 0;    
    msgOut(doorStatLEDPin, 0);
  }

  Blynk.virtualWrite(1, d1S);  //sends the state of Garage door 1 - if 1, then open
  Blynk.virtualWrite(3, d2S);  // sends the state of Garage door 2 - if 1, then open  
  
 
  //reset the timer values used for pwd entry
  if (millis() - pwdEntryStartTime > interval)
  {
    door_p1 = false, door_p2 = false, door_p3 = false, door_p4 = false;
    //reset the sliders in the Blynk app - do this after the interval time has elapsed.
	Blynk.virtualWrite(9, 0);  //pwd is no longer valid - update LED to OFF state
    Blynk.virtualWrite(5, 0);  //reset the pwd sliders to 0
    Blynk.virtualWrite(6, 0);  //reset the pwd sliders to 0  
    Blynk.virtualWrite(7, 0);  //reset the pwd sliders to 0
    Blynk.virtualWrite(8, 0);  //reset the pwd sliders to 0      
    pwdEntryStartTime = 0;
  }
  
  if (door_p1 && door_p2 && door_p3 && door_p4)
  {
    Blynk.virtualWrite(9, 1);  //pwd is correct - illuminate LED
  }
  else
  {
    Blynk.virtualWrite(9, 0);  //pwd is no longer correct - illuminate LED    
  }
  
    
}

//************************************************************************************************************************************************
// a state of 0 means one or more doors open. Else, 1 means all good
// Uses AnalogRead on slave ATTiny85 to determine which state was sent and blink either a red or green LED.
void msgOut(int statLED, int state)
{
 if (state == 1)
 {
   //red LED - one or more doors are open
   //analogWrite(statLED, 255);
   digitalWrite(statLED, HIGH);
 }
 else
 {
   //green LED - everything good
   //analogWrite(statLED, 0);
   digitalWrite(statLED, LOW);
 } 
}
//************************************************************************************************************************************************
// DOOR 1
// This function will be called every time when the Blynk App writes value to Virtual Pin 0  - This will be when Door 1 is clicked to init closing the door
//Note - when App Virtual Button is set as Push - it will send 2 commands, first HIGH, then LOW one right afer each other. 
BLYNK_WRITE(0)
{
  //BLYNK_LOG("Got a value: %s", param.asStr());
  // You can also use: asInt() and asDouble()
  //digitalWrite(OKLedPin, param.asInt());
  //label = 1;  //tell it to act on door 2
  //Serial.print("Param:  ");
  //Serial.println(param.asInt());

  
  if (param.asInt() == 1 && (door_p1 && door_p2 && door_p3 && door_p4))  //passcode must be true.  if so and clicks button open door.
  {  
    if (millis() - pwdEntryStartTime < interval)
    {
      Action(1);   //send command to master to open/close door.
    }  
    //door_p1 = false, door_p2 = false, door_p3 = false, door_p4 = false;
  }  
}
//************************************************************************************************************************************************
// DOOR 2
// This function will be called every time when the Blynk App writes value to Virtual Pin 2  - This will be when Door 2 is clicked to init closing the door
BLYNK_WRITE(2)
{
  //BLYNK_LOG("Got a value: %s", param.asStr());
  // You can also use: asInt() and asDouble()
  //digitalWrite(OKLedPin, param.asInt());
  if (param.asInt() == 1 && (door_p1 && door_p2 && door_p3 && door_p4))  //passcode must be true.  if so and clicks button open door.
  {  
   if (millis() - pwdEntryStartTime < interval)
   {
     Action(2);   //send command to master to open/close door.
   }
    //door_p1 = false, door_p2 = false, door_p3 = false, door_p4 = false;
  }
}
//************************************************************************************************************************************************


// Door 1 passcode values - uses sliders in Blynk
BLYNK_WRITE(5)
{
  pwdEntryStartTime = millis();  //start the timer 
  //Blynk.virtualWrite(9, 1);  //illuminate the led to indicate pwd entry is valid.
  if (param.asInt() == 1) {  door_p1 = true; } else  { door_p1 = false; }  
}
BLYNK_WRITE(6)
{
  if (param.asInt() == 1) {  door_p2 = true; } else  {    door_p2 = false; }  
}
BLYNK_WRITE(7)
{
  if (param.asInt() == 1) {  door_p3 = true; } else  {    door_p3 = false; }  
}
BLYNK_WRITE(8)
{
  if (param.asInt() == 1) {  door_p4 = true; } else  {    door_p4 = false; }  
}

//************************************************************************************************************************************************

void Action(int label)
{
  //digitalWrite(OKLedPin, HIGH);
  if(label == 1)
  {
    digitalWrite(door1ClosePin, HIGH);delay(200);digitalWrite(door1ClosePin,LOW);   
  }
  if(label == 2)
  {
    digitalWrite(door2ClosePin, HIGH); delay(200); digitalWrite(door2ClosePin, LOW);
  }
  delay(500);
  //digitalWrite(OKLedPin, LOW);
}
//************************************************************************************************************************************************