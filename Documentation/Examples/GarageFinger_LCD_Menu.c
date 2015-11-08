/*********************************************************************
This script is part of my fingerprint garage door opener.
This is for the ATmega that controls the FPS and a display screen
Upon authorizing, it sends a secret code to an ATtiny which
validates it and opens/shuts the garage door.
*********************************************************************/
#include <EEPROM.h>
#include "FPS_GT511C1R.h"
#include "SoftwareSerial.h"
#include "NokiaLCD.h"

//      NokiaLCD( reset, sce, dc, sdin, sclk)
NokiaLCD display(10, 9, 11, 12, 13);

//      FPS_GT511C1R( tx, rx )
FPS_GT511C1R fps(4,2);

const int backLightCtrl=3;
const int upButton=8;
const int okButton=7;
const int dnButton=6;
const int fpsTouch=100;
const int TIMEOUT=99;
int backLightVal=10;
int contrastVal=5;
int userid;
char overrideCode=172; // 10101100

// wait for a button to be pressed
// and/or FPS touch
// return the code for the button
// if nothing happens for a bit,
// return a timeout code
int waitForButton(bool waitscan=0) {
  int id=0;
  int N=400;
  if (waitscan) N=80; // FPS takes a while to check
  for (int t=0; t<N; t++) {
    if (digitalRead(upButton)==HIGH) {
      id=upButton;
    } else if (digitalRead(dnButton)==HIGH) {
      id=dnButton;
    } else if (digitalRead(okButton)==HIGH) {
      id=okButton;
    } else if (waitscan && fps.IsPressFinger()) {
       return fpsTouch;
    } else {
      id=0;
    }
    delay(20);
    if (id==upButton && digitalRead(upButton)==LOW) {
      return upButton;
    } else if (id==dnButton && digitalRead(dnButton)==LOW) {
      return dnButton;
    } else if (id==okButton && digitalRead(okButton)==LOW) {
      return okButton;
    }
  }
  return TIMEOUT;
}

int menuChoice(char* items[], int n) {
  int opt=0;
  display.clear();
  while (1) {
    display.gotoXY(0,0);
    for (int i=0;i<n;i++) {
      if (i==opt) {
        display.print(items[i],LCD_INVERSE);
      } else {
        display.print(items[i]);
      }
    }
    int button=waitForButton(0);
    if (button==TIMEOUT) {
      break;
    } else if (button==upButton) {
      opt--;
      if (opt<0) {
        opt=n-1;
      }
    } else if (button==dnButton) {
      opt++;
      if (opt>n-1) {
        opt=0;
      }
    } else if (button==okButton) {
      display.clear();
      return opt;
    }
  }
  display.clear();
  return TIMEOUT;
}

void doEnroll(int n) {
  int m;
  if (fps.EnrollStart(n)>0) {
    display.clear();
    display.print("Unable to   enroll");
    delay(3000);
    return;
  }
  // Do 1st scan
  display.clear();
  display.print("Enrolling   user ");
  display.print(n);
  display.gotoXY(3,14);
  display.print("Scan 1/3");
  fps.SetLED(HIGH);
  m=waitForButton(1);
  if (m != fpsTouch) {
    fps.SetLED(LOW);
    display.clear();
    display.print("Enrollment  canceled");
    delay(3000);
    return;
  }
  if (!fps.CaptureFinger(1)) {
    fps.SetLED(LOW);
    display.clear();
    display.print("Scan failed");
    delay(3000);
    return;
  }
  fps.SetLED(LOW);
  if (fps.Enroll1()>0) {
    display.clear();
    display.print("Enrollment  failed");
    delay(3000);
  }
  while(fps.IsPressFinger()) {
    delay(100);
  }
  // Do 2nd scan
  display.clear();
  delay(1000);
  display.print("Enrolling   user ");
  display.print(n);
  display.gotoXY(3,14);
  display.print("Scan 2/3");
  fps.SetLED(HIGH);
  m=waitForButton(1);
  if (m != fpsTouch) {
    fps.SetLED(LOW);
    display.clear();
    display.print("Enrollment  canceled");
    delay(3000);
    return;
  }
  if (!fps.CaptureFinger(1)) {
    fps.SetLED(LOW);
    display.clear();
    display.print("Scan failed");
    delay(3000);
    return;
  }
  fps.SetLED(LOW);
  if (fps.Enroll2()>0) {
    display.clear();
    display.print("Enrollment  failed");
    delay(3000);
  }
  while(fps.IsPressFinger()) {
    delay(100);
  }
  // Do 3rd scan
  display.clear();
  delay(1000);
  display.print("Enrolling   user ");
  display.print(n);
  display.gotoXY(3,14);
  display.print("Scan 3/3");
  fps.SetLED(HIGH);
  m=waitForButton(1);
  if (m != fpsTouch) {
    fps.SetLED(LOW);
    display.clear();
    display.print("Enrollment  canceled");
    delay(3000);
    return;
  }
  if (!fps.CaptureFinger(1)) {
    fps.SetLED(LOW);
    display.clear();
    display.print("Scan failed");
    delay(3000);
    return;
  }
  fps.SetLED(LOW);
  if (fps.Enroll3()>0) {
    display.clear();
    display.print("Enrollment  failed");
    delay(3000);
  } else {
    display.clear();
    display.print("User ");
    display.print(n);
    display.gotoXY(1,0);
    display.print("Enrollment  successful");
    delay(3000);
  }
}

void accessMenu(int userid) {
  int m;
  int n;
  char* items1[]={"Open/Close  ","Settings    ","Exit        "};
  char* items2[]={"Users       ","Display     ","Set Code    ","Done        "};
  char* items3[]={"Add New     ","Replace     ","Delete      ","Back        "};
  char* items4[]={"Brightness  ","Contrast    ","Back        "};
  while (1) {;
    m=menuChoice(items1,3);
    if (m==2 || m==TIMEOUT) {
      return;
    } else if (m==0) {
      Serial.print("secretstring");
      delay(2000);
    } else if (m==1) {  // settings menu
      while (1) {
        m=menuChoice(items2,4);
        if (m==TIMEOUT) {
          return;
        } else if (m==3) {
          break;
        } else if (m==0) { // fingerprints menu
          while (1) {
            m=menuChoice(items3,4);
            if (m==TIMEOUT) return;
            else if (m==3) break;
            else if (m==0) {
              n=0;
              for (n=0;n<20;n++) {
                if (!fps.CheckEnrolled(n)) break;
              }
              if (n>19) {
                display.print("Database    full!");
                delay(3000);
                break;
              }
              doEnroll(n);
            } else if (m==1 && userid<20) {
              display.print("Replacing   fingerprint for user ");
              display.print(userid);
              delay(3000);
              fps.DeleteID(userid);
              doEnroll(userid);
            } else if (m==2) {
              if (fps.GetEnrollCount()<2) {
                display.print("Cannot      delete");
                delay(3000);
                break;
              } else {
                n=0;
                while (1) {
                  while (1) {
                    if (n==userid) n++;
                    if (n>19) n=0;
                    if (fps.CheckEnrolled(n)) break;
                    n++;
                  }
                  display.clear();
                  display.print("Delete user ");
                  display.print(n,LCD_INVERSE);
                  m=waitForButton(0);
                  if (m==TIMEOUT) return;
                  else if (m==dnButton) break;
                  else if (m==upButton) n++;
                  else if (m==okButton) {
                    display.clear();
                    display.print("User ");
                    display.print(n);
                    display.gotoXY(2,0);
                    display.print("Press OK    to delete");
                    m=waitForButton(0);
                    if (m==TIMEOUT) return;
                    else if (m!=okButton) {
                      display.clear();
                      display.print("Delete      canceled");
                      delay(3000);
                      break;
                    } else {
                      fps.DeleteID(n);
                      display.clear();
                      display.print("User ");
                      display.print(n);
                      display.gotoXY(1,0);
                      display.print("deleted");
                      delay(3000);
                      break;
                    }
                  }
                }
              }
            }
          }
        } else if (m==1) { // display menu
          while (1) {
            m=menuChoice(items4,3);
            if (m==TIMEOUT) return;
            else if (m==2) break;
            else if (m==0) {
              while (1) {
                display.clear();
                display.print("Brightness: ");
                display.print(backLightVal,LCD_INVERSE);
                m=waitForButton(0);
                if (m==TIMEOUT) return;
                if (m==okButton) break;
                else if (m==upButton) backLightVal+=1;
                else if (m==dnButton) backLightVal-=1;
                if (backLightVal>10) backLightVal=10;
                if (backLightVal<0) backLightVal=0;
                analogWrite(backLightCtrl, backLightVal*25);
              }
            } else if (m==1) {
              while (1) {
                display.clear();
                display.print("Contrast:   ");
                display.print(contrastVal,LCD_INVERSE);
                m=waitForButton(0);
                if (m==TIMEOUT) return;
                if (m==okButton) break;
                else if (m==upButton) contrastVal+=1;
                else if (m==dnButton) contrastVal-=1;
                if (contrastVal>15) contrastVal=15;
                if (contrastVal<0) contrastVal=0;
                display.setContrast(contrastVal);
              }
            }
          }
        } else if (m==2) {
          // Reset override code
          display.clear();
          display.print("Enter code: ");
          char code1=0x00;
          for (int i=0;i<8;i++) {
            n=waitForButton(0);
            if (n==upButton) {
              code1 = (code1<<1);
              code1 |= 1;
              display.print("1");
            } else if (n==dnButton) {
              code1 = (code1<<1);
              display.print("0");
            } else if (n==TIMEOUT) {
              return;
            }
          }
          display.gotoXY(2,0);
          display.print("Repeat code:");
          char code2=0x00;
          for (int i=0;i<8;i++) {
            n=waitForButton(0);
            if (n==upButton) {
              code2 = (code2<<1);
              code2 |= 1;
              display.print("1");
            } else if (n==dnButton) {
              code2 = (code2<<1);
              display.print("0");
            } else if (n==TIMEOUT) {
              return;
            }
          }
          if (code1!=code2) {
            display.gotoXY(4,0);
            display.print("No match!");
            delay(3000);
            break;
          } else {
            display.clear();
            display.print("Override    code changed");
            overrideCode=code1;
            EEPROM.write(0,overrideCode);
            delay(3000);
            break;
          }   
        }
      }
    }        
  }
}

// FPS lights up, waits for a good scan
// also you can manually enter code
// with up-down press combo
void wakeUp() {
  int button;
  char code=0x00;
  display.clear();
  display.print("Press fingeron scanner");
  fps.SetLED(HIGH);
  while (1) {
    button=waitForButton(1);      
    if (button==TIMEOUT) {
      return; // took too long, go back
    } else if (button==fpsTouch) {
      fps.CaptureFinger(false);
      userid = fps.Identify1_N();
      if (userid==200) {
        display.clear();
        display.print("Not         recognized");
        fps.SetLED(LOW);
        delay(3000);
        display.clear();
        display.print("Press fingeron scanner");
        fps.SetLED(HIGH);
      } else {
        display.clear();
        display.print("User ");
        display.print(userid);
        display.gotoXY(1,0);
        display.print("access      granted");
        Serial.print("secretstring");
        fps.SetLED(LOW);
        delay(200);
        fps.SetLED(HIGH);
        delay(200);
        fps.SetLED(LOW);
        delay(200);
        fps.SetLED(HIGH);
        delay(200);
        fps.SetLED(LOW);
        delay(200);
        fps.SetLED(HIGH);
        delay(200);
        fps.SetLED(LOW);
        delay(800);
        accessMenu(userid);
        return;
      }
    } else if (button==upButton) {
      code = (code<<1);
      code |= 1;
    } else if (button==dnButton) {
      code=(code<<1);
    } else if (button=okButton) {
      if (code == overrideCode) {
        fps.SetLED(LOW);
        display.clear();
        display.print("Override    access      granted");
        Serial.print("secretstring");
        analogWrite(backLightCtrl, 0);
        delay(200);
        analogWrite(backLightCtrl, backLightVal*25);
        delay(200);
        analogWrite(backLightCtrl, 0);
        delay(200);
        analogWrite(backLightCtrl, backLightVal*25);
        delay(200);
        analogWrite(backLightCtrl, 0);
        delay(200);
        analogWrite(backLightCtrl, backLightVal*25);
        delay(1000);
        accessMenu(99);
        return;
      }
      code=0;
    }
  }
}

void setup() {
  pinMode(upButton, INPUT);
  pinMode(dnButton, INPUT);
  pinMode(okButton, INPUT);
  pinMode(backLightCtrl, OUTPUT);

  display.init();
  display.setContrast(0);
  display.clear();

  char code=EEPROM.read(0);
  if (code!=255) overrideCode=code;

  fps.Open();
  
  Serial.begin(9600);
}

// idles with display and FPS led off
// until someone presses a button
// OR just goes straight to wakeup if I
// decide to build this with a power latch
// on the cover
void loop() {
  if (waitForButton(0) != TIMEOUT) {
    analogWrite(backLightCtrl, backLightVal*25);
    display.setContrast(contrastVal);
    wakeUp();
    fps.SetLED(LOW); // just in case
    display.clear();
    display.setContrast(0);
    analogWrite(backLightCtrl, 0);
  }
}

