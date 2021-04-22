#include <../libraries/SPI/SPI.h> // Why is path needed? Maybe something wrong with include paths for this project
#include <Arduino.h>
#include "HardwareSerial.h"
#include <tm4c123gh6pm.h>

#define CS PA_2
#define STEP PA_6
#define SLEEPn PE_5
#define DIR PA_7
#define RST PA_5
#define STALL PB_2
#define FAULT PE_0
#define uSec 20

void Init();
unsigned int WriteSPI_new(unsigned char dataHi, unsigned char dataLo);
unsigned int ReadSPI(unsigned char readRegister, unsigned char dataLo);
void getMotorDriverRegisters();
void push1();
void stop();
void poll_serial();

SPIClass mySPI(2); //SSI2 for tm4c123

volatile bool go = false;
volatile bool stop_motor = false;
volatile char command;
void setup()
{
  pinMode(PUSH1, INPUT_PULLUP);
  pinMode(PUSH2, INPUT_PULLUP);
  pinMode(PE_4, OUTPUT);
  attachInterrupt(PUSH1, push1, RISING);
  attachInterrupt(PUSH2, stop, RISING);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  //output pins
  pinMode(CS, OUTPUT);//OLD MSP430 CODE --> P8DIR |= CS;
  
  pinMode(STEP, OUTPUT); //STEP signal 
  pinMode(DIR, OUTPUT); //DIR signal

  pinMode(RST, OUTPUT); //RESET
  pinMode(STALL, OUTPUT); //STALL
  pinMode(FAULT, OUTPUT); //FAULT

  pinMode(SLEEPn, OUTPUT); //SLEEP
  digitalWrite(FAULT, HIGH); // fault input
  digitalWrite(STALL, HIGH); // Stall input
  digitalWrite(SLEEPn, HIGH); //nSLEEP = high
  digitalWrite(DIR, HIGH); //dir = high
  digitalWrite(RST, LOW);  //reset = low
  
  mySPI.begin();
  mySPI.setClockDivider(SPI_CLOCK_DIV2);
  digitalWrite(CS, LOW);
  delay(50);
  Init();
  Serial.begin(9600);
  getMotorDriverRegisters();
}


void loop()
{
  poll_serial();

  if(go)
  {
    digitalWrite(GREEN_LED, LOW); //not ready
    digitalWrite(PE_4, HIGH); // TURN ON BJT which is the open collector output 
    for(int i=0; i<200*72*11; i++){ //Currently at 1/32 micro stepping. 16 would be 180 degree rotation
      digitalWrite(STEP, LOW); // step
      delayMicroseconds(uSec);
      digitalWrite(STEP, HIGH);// step
      poll_serial(); //Ideally we don't want to be polling for a stop character in here. Can we make it an interrupt. Stop rotary when 's' character is received.
      delayMicroseconds(20);
    }
    go = false;
    digitalWrite(RED_LED, LOW); //if the stop light is on turn it off
    digitalWrite(GREEN_LED, HIGH); //ready
    digitalWrite(PE_4, LOW);

  }
  //WriteSPI_new(0x0C, 0x1D);
  
  //WriteSPI_new(0x0C, 0x04);
  //delay(4000);
  /*
  WriteSPI_new(0x0C, 0x07); //reverse the direction to return back
  for(int i=0; i<200*72; i++){
      digitalWrite(STEP, LOW);
      delayMicroseconds(uSec);
      digitalWrite(STEP, HIGH);
      delayMicroseconds(uSec);
  }
  WriteSPI_new(0x0C, 0x04);
  delay(1000);
  */
}

void poll_serial()
{
  while(Serial.available())
  {
    command = Serial.read();
    if(command != '\n' && command == 'g')
    {
      push1();
      Serial.println("Aye");
    }
    else if(command == 's')
    {
      stop();
      Serial.println("Stop");
    }
  }
}

void push1()
{
  if(!go)
  {
    go = true;
    WriteSPI_new(0x0D, 0x2D);
  }
}

void stop()
{
  WriteSPI_new(0x0D, 0x2C);
  digitalWrite(RED_LED, HIGH);
}

void rx_handler()
{
  while(Serial.available())
  {
    command = Serial.read();
    if(command != '\n' && command == 'g')
    {
      go = true;
      Serial.println("Aye");
    }
  }
}

void Init()
{
  //CTRL Register defaults
  unsigned char CTRLdataHi, CTRLdataLo;
  CTRLdataHi = 0x0D; // 0000_1101 --> ISGAIN = 10(decimal)
  CTRLdataLo = 0x2D;//0x05; 0b0010_1101
  WriteSPI_new(CTRLdataHi, CTRLdataLo);

  //TORQUE defaults
  unsigned char TORQUEHi, TORQUELo;
  TORQUEHi = 0x13;
  TORQUELo = 0x50;//0x82;//0x1F;//0x41;
  WriteSPI_new(TORQUEHi, TORQUELo);

  //OFF defaults
  unsigned char OFFHi, OFFLo;
  OFFHi = 0x20;
  OFFLo = 0x33;//0x30;
  WriteSPI_new(OFFHi, OFFLo);

  //BLANK defaults
  unsigned char BLNKHi, BLNKLo;
  BLNKHi = 0x31;
  BLNKLo = 0x30;//0xF0;
  WriteSPI_new(BLNKHi, BLNKLo);

  //DECAY defaults
  unsigned char DECAYHi, DECAYLo;
  DECAYHi = 0x43;//0x45;//0x41;
  DECAYLo = 0x10;
  WriteSPI_new(DECAYHi, DECAYLo);

  //STALL defaults
  unsigned char STALLHi, STALLLo;
  STALLHi = 0x53;
  STALLLo = 0x40;
  WriteSPI_new(STALLHi, STALLLo);

  //DRIVE defaults
  unsigned char DRIVEHi, DRIVELo;
  DRIVEHi = 0x60;
  DRIVELo = 0x0F;
  WriteSPI_new(DRIVEHi, DRIVELo);

  //STATUS defaults
  unsigned char STATUSHi, STATUSLo;
  STATUSHi = 0x70;
  STATUSLo = 0x00;
  WriteSPI_new(STATUSHi, STATUSLo);
}

unsigned int WriteSPI_new(unsigned char dataHi, unsigned char dataLo)
{
  
  unsigned int readData = 0;
  digitalWrite(CS, HIGH);
  mySPI.transfer(dataHi);
  
  mySPI.transfer(dataLo);
  digitalWrite(CS, LOW);

  return readData;
}

void getMotorDriverRegisters()
{
  Serial.print("\n-----------------------------------------------\n");
  Serial.print("CTRL register ");
  Serial.print(ReadSPI(0x80, 0x00),HEX);
  Serial.print("\n");

  Serial.print("TORQUE register ");
  Serial.print(ReadSPI(0x90, 0x00),HEX);
  Serial.print("\n");

  Serial.print("OFF register ");
  Serial.print(ReadSPI(0xA0, 0x00),HEX);
  Serial.print("\n"); 

  Serial.print("BLANK register ");
  Serial.print(ReadSPI(0xB0, 0x00),HEX);
  Serial.print("\n");

  Serial.print("DECAY register ");
  Serial.print(ReadSPI(0xC0, 0x00),HEX);
  Serial.print("\n");

  Serial.print("STALL register ");
  Serial.print(ReadSPI(0xD0, 0x00),HEX);
  Serial.print("\n");

  Serial.print("DRIVE register ");
  Serial.print(ReadSPI(0xE0, 0x00),HEX);
  Serial.print("\n");

  Serial.print("STATUS register ");
  Serial.print(ReadSPI(0xF0, 0x00),HEX);
  Serial.print("\n-----------------------------------------------\n");
}

unsigned int ReadSPI(unsigned char readRegister, unsigned char dataLo)
{
  
  unsigned int result = WriteSPI_new(readRegister, 0x00);//0;
  digitalWrite(CS, HIGH);
  result = (mySPI.transfer(readRegister) << 8);
  result |= mySPI.transfer(0x00);
  result &= 0x0FFF;
  digitalWrite(CS, LOW);
  
/*
  unsigned int readData = 0;

  digitalWrite(CS,HIGH);

  SSI2_DR_R = readRegister;
  while ((SSI2_SR_R & (1<<0)) == 0);
  readData = (SSI2_DR_R<<8);

  SSI2_DR_R = 0x00;
  while ((SSI2_SR_R & (1<<0)) == 0);
  readData |= (SSI2_DR_R);
  delayMicroseconds(30);
  */
  /*
  SSI2_DR_R = readRegister;
  while ((SSI2_SR_R & (1<<0)) == 0);
  readData |= SSI2_DR_R;
  delayMicroseconds(100);
  */
  digitalWrite(CS, LOW);

  return result;
}