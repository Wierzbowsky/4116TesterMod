//
// 4116 DRAM Tester Firmware. v3.0
//
// Created in November 2017. Code commented and posted in March 2020. 
// Most of the code and design is from http://forum.defence-force.org/viewtopic.php?p=15035&sid=17bf402b9c2fd97c8779668b8dde2044
// by forum member "iss" and modified to work with 4116 D ram by me Uffe Lund-Hansen, Frostbox Labs. 
//
// Mod: The firmware was modified by Wierzbowsky [RBSC] in September 2021. Mod v1.3
//
// Board: v3.0 with the push button disconnected from RST and connected to A6 of Arduino Nano
//
// The additional relay module can be connected to the board to enable the power on the
// chip's socket only during DRAM's testing, the control signal is wired to D12 of Arduino Nano
//
// An OLED screen can be connected to the A4 and A5 pins of Arduino Nano (SDA and SCL)
// The previous connections from A4 and A5 to resistors should be rewired to D10 and D11 respectively!
// This mod and firmware is designed for 128x64 SSD1306 OLED screen, the serial output is disabled when screen is used
//

#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
#define I2C_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//Adafruit_SSD1306 display(OLED_RESET);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DI          7     //  Arduino D7  > 4116 pin 2
#define DO          15    //  Arduino A1  > 4116 pin 14
#define CAS         14    //  Arduino A0  > 4116 pin 15
#define RAS         5     //  Arduino D5  > 4116 pin 4
#define WE          6     //  Arduino D6  > 4116 pin 3

#define XA0         4     //  Arduino D4  > 4116 pin 5
#define XA1         2     //  Arduino D2  > 4116 pin 7
#define XA2         3     //  Arduino D3  > 4116 pin 6
#define XA3         17    //  Arduino A3  > 4116 pin 12
//#define XA4         18    //  Arduino A4 > 4116 pin 11
//#define XA5         19    //  Arduino A5 > 4116 pin 10
#define XA4         10    //  Arduino D10 > 4116 pin 11
#define XA5         11    //  Arduino D11 > 4116 pin 10
#define XA6         16    //  Arduino A2  > 4116 pin 13

#define SDA         18    //  Arduino A4 > screen SDA
#define SCL         19    //  Arduino A5 > screen SCL

#define R_LED       9     //  Arduino D9 > Cathode on red LED. Anode connected through 470ohm resisor to +5VDC 
#define G_LED       8     //  Arduino D8 > Cathode on green LED. Anode connected through 470ohm resisor to +5VDC

#define BUTN        20    //  Arduino A6  > Start button
#define PWR         12    //  Arduino D12 > Socket power on/off via relay
#define NLED        13    //  Arduino D13 > NanoLED

#define BUS_SIZE    7

volatile int bus_size;

int buttondown;

const unsigned int a_bus[BUS_SIZE] = {
  XA0, XA1, XA2, XA3, XA4, XA5, XA6
};

void setBus(unsigned int a) {
  int i;
  for (i = 0; i < BUS_SIZE; i++) {
    digitalWrite(a_bus[i], a & 1);
    a /= 2;
  }
}

void writeAddress(unsigned int r, unsigned int c, int v) {
  /* row */
  setBus(r);
  digitalWrite(RAS, LOW);

  /* rw */
  digitalWrite(WE, LOW);

  /* val */
  digitalWrite(DI, (v & 1) ? HIGH : LOW);

  /* col */
  setBus(c);
  digitalWrite(CAS, LOW);

  digitalWrite(WE, HIGH);
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
}

int readAddress(unsigned int r, unsigned int c) {
  int ret = 0;

  /* row */
  setBus(r);
  digitalWrite(RAS, LOW);

  /* col */
  setBus(c);
  digitalWrite(CAS, LOW);

  /* get current value */
  ret = digitalRead(DO);

  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);

  return ret;
}

void TestFailed(int r, int c)
{
  unsigned long a = ((unsigned long)c << bus_size) + r;
  digitalWrite(R_LED, LOW);
  digitalWrite(G_LED, HIGH);
  digitalWrite(NLED, 1); // Nano led on
  digitalWrite(PWR, LOW); // power off

  interrupts();
//  Serial.println();
//  Serial.print("TEST FAILED at 0x");
//  Serial.println(a, HEX);
//  Serial.println("Press a button to acknowledge...");
//  Serial.flush();

  ClearSpace();
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.setCursor(4,40);
  display.println("    TEST FAILED!!   ");
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2,50);
  display.println(" Push button to ack.");
  display.display();
}

void TestOK(void)
{
  digitalWrite(R_LED, HIGH);
  digitalWrite(G_LED, LOW);
  digitalWrite(NLED, 0); // Nano led off
  digitalWrite(PWR, LOW); // power off
  
  interrupts();  
//  Serial.println();
//  Serial.println("TEST PASSED!");
//  Serial.println("Press a button to acknowledge...");
//  Serial.flush();

  ClearSpace();
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.setCursor(4,40);
  display.println("     TEST PASSED    ");
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2,50);
  display.println(" Push button to ack.");
  display.display();
}

void BlinkLEDs(void)
{
  digitalWrite(G_LED, LOW);
  digitalWrite(R_LED, LOW);
  digitalWrite(NLED, 1); // Nano led on
  delay(1000);
  digitalWrite(R_LED, HIGH);
  digitalWrite(G_LED, HIGH);
  digitalWrite(NLED, 0); // Nano led off
}

void SetGreen(int v)
{
  digitalWrite(G_LED, v);
}

boolean Test2(int v)
{
  int r, c, g = 0;
  v &= 1;
  for (c = 0; c < (1 << bus_size); c++) {
    SetGreen(g ? HIGH : LOW);
    for (r = 0; r < (1 << bus_size); r++) {
      writeAddress(r, c, v);
      if (v != readAddress(r, c))
      {
        TestFailed(r, c);
        return false;
      }
    }
    g ^= 1;
  }
  BlinkLEDs();
  return true;
}

boolean Test1(int v)
{
  int r, c, g = 0;
  v &= 1;
  for (c = 0; c < (1 << bus_size); c++) {
    SetGreen(g ? HIGH : LOW);
    for (r = 0; r < (1 << bus_size); r++) {
      writeAddress(r, c, v);
      if (v != readAddress(r, c))
      {
        TestFailed(r, c);
        return false;
      }
      v ^= 1;
    }
    g ^= 1;
  }
  BlinkLEDs();
  return true;
}

int ButtonCheck(void)
{
  while(1)
  {
    delay(10);
    buttondown = analogRead(BUTN);
    if(buttondown) return buttondown;
    else while(1)
    {
      delay(10);
      buttondown = analogRead(BUTN);
      if(buttondown) return 0;
    }
  }
}

void ClearSpace()
{
  display.fillRect(1, 32, 126, 31, SSD1306_BLACK);
  display.display();
}

void setup() {
  int i;

  Serial.begin(9600);
  while (!Serial)
    ; /* wait */

  digitalWrite(NLED, 1); // Nano led on

  display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.cp437(true);
  display.setTextColor(SSD1306_WHITE);

//  Serial.println();
//  Serial.println("4116 DRAM TESTER v1.3 MOD by Wierzbowsky [RBSC]");
//  Serial.println("Setting up...");

  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.setCursor(4,5);
  display.println("  4116 DRAM TESTER");
  display.setCursor(4,14);
  display.println(" MOD version 1.3 by");
  display.setCursor(4,23);
  display.println(" Wierzbowsky [RBSC]");
  display.setCursor(4,40);
  display.println("    Setting up...");
  display.display();
  delay(1000);

  for (i = 0; i < BUS_SIZE; i++)
    pinMode(a_bus[i], OUTPUT);
  
  pinMode(CAS, OUTPUT);
  pinMode(RAS, OUTPUT);
  pinMode(WE, OUTPUT);
  pinMode(DI, OUTPUT);

  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);

  pinMode(DO, INPUT);
  
  pinMode(BUTN, INPUT_PULLUP);  // for Start button (connected to D11)
  
  pinMode(PWR, OUTPUT);  // for power on/off
  
  digitalWrite(PWR, LOW);
  
  digitalWrite(WE, HIGH);
  digitalWrite(RAS, HIGH);
  digitalWrite(CAS, HIGH);

  digitalWrite(R_LED, LOW);
  digitalWrite(G_LED, LOW);

  bus_size = BUS_SIZE;

  noInterrupts();
  for (i = 0; i < (1 << BUS_SIZE); i++) {
    digitalWrite(RAS, LOW);
    digitalWrite(RAS, HIGH);
  }

  digitalWrite(NLED, 0); // Nano led off
  interrupts();
}

void loop()
{
  const int divider = 10;
  int cnt;

  digitalWrite(PWR, LOW); // power off

//  Serial.println("Ready for testing. Press a button to start...");
//  Serial.flush();

  while(1)  // main waiting loop
  {
    ClearSpace();
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // inverse
    display.setCursor(4,40);
    display.println("   PUSH BUTTON TO   ");
    display.setCursor(4,48);
    display.println("  TO START TESTING  ");
    display.setTextColor(SSD1306_WHITE);
    display.display();

    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, LOW);
    digitalWrite(NLED, 1); // Nano led on
    
    for (cnt = 0; cnt < divider; cnt++)
    {
      delay(100);
      buttondown = ButtonCheck();
      if (!buttondown) break;
    }
    if (!buttondown) break;

    ClearSpace();
    display.setCursor(4,40);
    display.println("   PUSH BUTTON TO   ");
    display.setCursor(4,48);
    display.println("  TO START TESTING  ");
    display.display();

    digitalWrite(R_LED, HIGH);
    digitalWrite(G_LED, HIGH);
    digitalWrite(NLED, 0); // Nano led off
    
    for (cnt = 0; cnt < divider; cnt++)
    {
      delay(100);
      buttondown = ButtonCheck();
      if (!buttondown) break;
    }
    if (!buttondown) break;
  }

  // LEDS off
  interrupts();
  digitalWrite(G_LED, HIGH);
  digitalWrite(R_LED, HIGH);
  digitalWrite(NLED, 0); // Nano led off

  // Start test
  interrupts();
//  Serial.print("Testing");
//  Serial.print(".");
//  Serial.flush();

  ClearSpace();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,40);
  display.println("       TESTING      ");
  display.drawRect(20, 50, 90, 10, SSD1306_WHITE);
  display.display();

  digitalWrite(PWR, HIGH);  // power on
  delay(1000);

  noInterrupts();
  if (Test1(0) == false)
  {
    noInterrupts();
    while(ButtonCheck());
    interrupts();
    return;
  }
  else
  {  
    interrupts();
//    Serial.print(".");
//    Serial.flush();
    display.fillRect(20, 50, 22, 10, SSD1306_WHITE);
    display.display();
    noInterrupts();
    if (Test1(1) == false)
    {
      while(ButtonCheck());
      interrupts();
      return;
    }
    else
    {
      interrupts();
//      Serial.print(".");
//      Serial.flush();
      display.fillRect(20, 50, 44, 10, SSD1306_WHITE);
      display.display();
      noInterrupts();
      if (Test2(0) == false)
      {
        while(ButtonCheck());
        interrupts();
        return;
      }
      else
      {
        interrupts();
//        Serial.print(".");
//        Serial.flush();
        display.fillRect(20, 50, 66, 10, SSD1306_WHITE);
        display.display();
        noInterrupts();
        if (Test2(1) == false)
        {
          while(ButtonCheck());
          interrupts();
          return;
        }
        else
        {
          interrupts();
          display.fillRect(20, 50, 90, 10, SSD1306_WHITE);
          display.display();
          delay(1000);
          
          // Test passed
          TestOK();
          while(ButtonCheck());
        }
      }
    }
  }
}
