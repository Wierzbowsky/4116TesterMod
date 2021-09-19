//
// 4116 DRAM Tester Firmware. v3.0
//
// Created in November 2017. Code commented and posted in March 2020. 
// Most of the code and design is from http://forum.defence-force.org/viewtopic.php?p=15035&sid=17bf402b9c2fd97c8779668b8dde2044
// by forum member "iss" and modified to work with 4116 D ram by me Uffe Lund-Hansen, Frostbox Labs. 
//
// This is the orignal firmware. For button, power and OLED screen mods please see version 1.2 and 1.3 of the mods
//

//#include <SoftwareSerial.h>

#define DI          7     //  Arduino D7 > 4116 pin 2
#define DO          15    //  Arduino A1 > 4116 pin 14
#define CAS         14    //  Arduino A0 > 4116 pin 15
#define RAS         5     //  Arduino D5 > 4116 pin 4
#define WE          6     //  Arduino D6 > 4116 pin 3

#define XA0         4     //  Arduino D4 > 4116 pin 5
#define XA1         2     //  Arduino D2 > 4116 pin 7
#define XA2         3     //  Arduino D3 > 4116 pin 6
#define XA3         17    //  Arduino A3 > 4116 pin 12
#define XA4         18    //  Arduino A4 > 4116 pin 11
#define XA5         19    //  Arduino A5 > 4116 pin 10
#define XA6         16    //  Arduino A2 > 4116 pin 13

#define R_LED       9     //  Arduino D9 > Cathode on red LED. Anode connected through 470ohm resisor to +5VDC 
#define G_LED       8     //  Arduino D8 > Cathode on green LED. Anode connected through 470ohm resisor to +5VDC



#define BUS_SIZE     7

volatile int bus_size;

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
  digitalWrite(DI, (v & 1)? HIGH : LOW);

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

void error(int r, int c)
{
  unsigned long a = ((unsigned long)c << bus_size) + r;
  digitalWrite(R_LED, LOW);
  digitalWrite(G_LED, HIGH);
  interrupts();
  Serial.print(" FAILED $");
  Serial.println(a, HEX);
  Serial.flush();
  while (1)
    ;
}

void ok(void)
{
  digitalWrite(R_LED, HIGH);
  digitalWrite(G_LED, LOW);
  interrupts();
  Serial.println(" OK!");
  Serial.flush();
  while (1)
    ;
}

void blink(void)
{
  digitalWrite(G_LED, LOW);
  digitalWrite(R_LED, LOW);
  delay(1000);
  digitalWrite(R_LED, HIGH);
  digitalWrite(G_LED, HIGH);
}

void green(int v) {
  digitalWrite(G_LED, v);
}


void fill(int v) {
  int r, c, g = 0;
  v &= 1;
  for (c = 0; c < (1<<bus_size); c++) {
    green(g? HIGH : LOW);
    for (r = 0; r < (1<<bus_size); r++) {
      writeAddress(r, c, v);
      if (v != readAddress(r, c))
        error(r, c);
    }
    g ^= 1;
  }
  blink();
}

void fillx(int v) {
  int r, c, g = 0;
  v &= 1;
  for (c = 0; c < (1<<bus_size); c++) {
    green(g? HIGH : LOW);
    for (r = 0; r < (1<<bus_size); r++) {
      writeAddress(r, c, v);
      if (v != readAddress(r, c))
        error(r, c);
      v ^= 1;
    }
    g ^= 1;
  }
  blink();
}

void setup() {
  int i;

  Serial.begin(9600);
  while (!Serial)
    ; /* wait */

  Serial.println();
  Serial.print("DRAM TESTER ");

  for (i = 0; i < BUS_SIZE; i++)
    pinMode(a_bus[i], OUTPUT);

  pinMode(CAS, OUTPUT);
  pinMode(RAS, OUTPUT);
  pinMode(WE, OUTPUT);
  pinMode(DI, OUTPUT);

  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);

  //pinMode(M_TYPE, INPUT);
  pinMode(DO, INPUT);

  digitalWrite(WE, HIGH);
  digitalWrite(RAS, HIGH);
  digitalWrite(CAS, HIGH);

  digitalWrite(R_LED, HIGH);
  digitalWrite(G_LED, HIGH);

//  if (digitalRead(M_TYPE)) {
//    /* jumper not set - 41256 */
    bus_size = BUS_SIZE;
//    Serial.print("256Kx1 ");
//  } else {
//    /* jumper set - 4164 */
//    bus_size = BUS_SIZE - 1;
//    Serial.print("64Kx1 ");
//  }
  Serial.flush();

  digitalWrite(R_LED, LOW);
  digitalWrite(G_LED, LOW);

  noInterrupts();
  for (i = 0; i < (1 << BUS_SIZE); i++) {
    digitalWrite(RAS, LOW);
    digitalWrite(RAS, HIGH);
  }
  digitalWrite(R_LED, HIGH);
  digitalWrite(G_LED, HIGH);
}

void loop() {
  interrupts(); Serial.print("."); Serial.flush(); noInterrupts(); fillx(0);
  interrupts(); Serial.print("."); Serial.flush(); noInterrupts(); fillx(1);
  interrupts(); Serial.print("."); Serial.flush(); noInterrupts(); fill(0);
  interrupts(); Serial.print("."); Serial.flush(); noInterrupts(); fill(1);
  ok();
}
