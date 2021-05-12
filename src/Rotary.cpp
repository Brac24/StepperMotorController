#include <Rotary.hpp>



Rotary::Rotary(SPIClass spiController)
{
    _spiController = spiController;

    //200 full steps for stepper motor
    //72:1 gear ratio
    //1/32 microstepping
    fullRev = 200*gearRatio*uStep;
    oneDegree = fullRev/360; // number of iterations needed to rotate 1 degree
}

void Rotary::rotate(int32_t moveDegrees)
{
  moveDegrees *= oneDegree;
  if(moveDegrees < 0)
  {
    digitalWrite(DIR1, HIGH); //dir = high
    moveDegrees *= -1; //Will this cause any inaccuracies if we decide to go with floats?
  }
  else
  {
    digitalWrite(DIR1, LOW); //dir = low
  }

    for(unsigned int i=0; i<moveDegrees; i++){ //Currently at 1/32 micro stepping. 16 would be 180 degree rotation
      digitalWrite(STEP1, LOW); // step
      delayMicroseconds(uSec1);
      digitalWrite(STEP1, HIGH);// step
      //poll_serial(); //Ideally we don't want to be polling for a stop character in here. Can we make it an interrupt. Stop rotary when 's' character is received.
      delayMicroseconds(uSec1);
      //getMotorDriverRegisters();
    }
}

void Rotary::Init()
{
  //CTRL Register defaults
  unsigned char CTRLdataHi, CTRLdataLo;
  CTRLdataHi = 0x0D; // 0000_1101 --> ISGAIN = 10(decimal)
  CTRLdataLo = 0x2D;//0x05; 0b0010_1101
  WriteSPI_new(CTRLdataHi, CTRLdataLo);

  //TORQUE defaults
  unsigned char TORQUEHi, TORQUELo;
  TORQUEHi = 0x10;
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
  DECAYHi = 0x45;//0x43;//0x41; //Use 0x45 (auto mixed decay) for ATI motor. Use 0x43 (mixed decay) for motor at home
  DECAYLo = 0x10;
  WriteSPI_new(DECAYHi, DECAYLo);

  //STALL defaults
  unsigned char STALLHi, STALLLo;
  STALLHi = 0x50;
  STALLLo = 0x80; //0x40
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

void Rotary::WriteSPI_new(unsigned char dataHi, unsigned char dataLo)
{
  digitalWrite(CS1, HIGH);
  _spiController.transfer(dataHi);
  
  _spiController.transfer(dataLo);
  digitalWrite(CS1, LOW);
}