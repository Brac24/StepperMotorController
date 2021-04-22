#include <../libraries/SPI/SPI.h> // Why is path needed? Maybe something wrong with include paths for this project
#include <Arduino.h>
#include "HardwareSerial.h"

void Initialize();
unsigned int WriteSPI(unsigned char dataHi, unsigned char dataLo);
void getCurrentRegisters();

#define CS PA_2
#define STEP PA_6
#define SLEEPn PE_5
#define DIR PA_7
#define RST PA_5
#define STALL PB_2
#define FAULT PE_0
int flag =0;
int i,j;

char in;
SPIClass old_mySPI(5);
void old_setup()
{
  //output pins
  pinMode(PA_2, OUTPUT);//OLD MSP430 CODE --> P8DIR |= CS;
  
  // //OLD MSP430 CODE --> P4DIR |= (STEP|DIR);
  pinMode(PA_6, OUTPUT); //STEP signal 
  pinMode(PA_7, OUTPUT); //DIR signal

  //OLD MSP430 CODE --> P2DIR |= (RST|STALL|FAULT);
  pinMode(PA_5, OUTPUT); //RESET
  pinMode(PB_2, OUTPUT); //STALL
  pinMode(PE_0, OUTPUT); //FAULT

  //OLD MSP430 CODE --> P6DIR |= SLEEPn;
  pinMode(PE_5, OUTPUT); //SLEEP
 // pinMode(P7_4, OUTPUT);
  digitalWrite(FAULT, HIGH); // fault input
  digitalWrite(STALL, HIGH); // Stall input
  digitalWrite(SLEEPn, HIGH); //nSLEEP = high
  digitalWrite(DIR, HIGH); //dir = high
  digitalWrite(RST, LOW);  //reset = low
  Serial.begin(9600);
  old_mySPI.begin();
  old_mySPI.setClockDivider(2);
  delay(1);
  Initialize();
  getCurrentRegisters(); 

  //digitalWrite(P7_4, HIGH);
  //delayMicroseconds(10);
  //digitalWrite(P7_4, LOW);
}

void old_loop()
{
  WriteSPI(0x0C, 0x05); // 1100_0000_0101
  for(i=0; i<200; i++){
      digitalWrite(STEP, LOW); // step
      delayMicroseconds(100);
      digitalWrite(STEP, HIGH);// step
      delayMicroseconds(10000);
  }
    WriteSPI(0x0C, 0x04);
  delay(1000);
  
  WriteSPI(0x0C, 0x07); //reverse the direction to return back
  for(i=0; i<200; i++){
      digitalWrite(STEP, LOW);
      delayMicroseconds(100);
      digitalWrite(STEP, HIGH);
      delayMicroseconds(10000);
  }
  WriteSPI(0x0C, 0x04);
  delay(1000);
}

unsigned int WriteSPI(unsigned char dataHi, unsigned char dataLo)
{
    unsigned int readData = 0;
    digitalWrite(CS, HIGH);
    old_mySPI.transfer(dataHi);
  //  
    old_mySPI.transfer(dataLo);
    digitalWrite(CS, LOW);

  /*
  unsigned int readData = 0;

  digitalWrite(CS,HIGH);

  UCB0TXBUF = dataHi;
  while (UCB0STAT & BUSY);
  readData |= (UCB0RXBUF << 8);

  UCB0TXBUF = dataLo;
  while (UCB0STAT & BUSY);
  readData |= UCB0RXBUF;
  digitalWrite(CS, LOW);
  readData &= 0x7FFF;
  */

  return readData;
}

void Initialize()
{
  //CTRL Register defaults
  unsigned char CTRLdataHi, CTRLdataLo;
  CTRLdataHi = 0x0C;
  CTRLdataLo = 0x05;
  WriteSPI(CTRLdataHi, CTRLdataLo);

  //TORQUE defaults
  unsigned char TORQUEHi, TORQUELo;
  TORQUEHi = 0x13;
  TORQUELo = 0x1F;
  WriteSPI(TORQUEHi, TORQUELo);

  //OFF defaults
  unsigned char OFFHi, OFFLo;
  OFFHi = 0x20;
  OFFLo = 0xF0;
  WriteSPI(OFFHi, OFFLo);

  //BLANK defaults
  unsigned char BLNKHi, BLNKLo;
  BLNKHi = 0x31;
  BLNKLo = 0xF0;
  WriteSPI(BLNKHi, BLNKLo);

  //DECAY defaults
  unsigned char DECAYHi, DECAYLo;
  DECAYHi = 0x41;
  DECAYLo = 0x10;
  WriteSPI(DECAYHi, DECAYLo);

  //STALL defaults
  unsigned char STALLHi, STALLLo;
  STALLHi = 0x53;
  STALLLo = 0x40;
  WriteSPI(STALLHi, STALLLo);

  //DRIVE defaults
  unsigned char DRIVEHi, DRIVELo;
  DRIVEHi = 0x60;
  DRIVELo = 0x0F;
  WriteSPI(DRIVEHi, DRIVELo);

  //STATUS defaults
  unsigned char STATUSHi, STATUSLo;
  STATUSHi = 0x70;
  STATUSLo = 0x00;
  WriteSPI(STATUSHi, STATUSLo);
}

void getCurrentRegisters()
{
  Serial.print("\n-----------------------------------------------\n");
  Serial.print("CTRL register ");
  Serial.print(WriteSPI(0x80, 0x00),HEX);
  Serial.print("\n");

  Serial.print("TORQUE register ");
  Serial.print(WriteSPI(0x90, 0x00),HEX);
  Serial.print("\n");

  Serial.print("OFF register ");
  Serial.print(WriteSPI(0xA0, 0x00),HEX);
  Serial.print("\n"); 

  Serial.print("BLANK register ");
  Serial.print(WriteSPI(0xB0, 0x00),HEX);
  Serial.print("\n");

  Serial.print("DECAY register ");
  Serial.print(WriteSPI(0xC0, 0x00),HEX);
  Serial.print("\n");

  Serial.print("STALL register ");
  Serial.print(WriteSPI(0xD0, 0x00),HEX);
  Serial.print("\n");

  Serial.print("DRIVE register ");
  Serial.print(WriteSPI(0xE0, 0x00),HEX);
  Serial.print("\n");

  Serial.print("STATUS register ");
  Serial.print(WriteSPI(0xF0, 0x00),HEX);
  Serial.print("\n-----------------------------------------------\n");
}