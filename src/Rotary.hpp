#include <cstdint>
#include <SPI.h>
#include <tm4c123gh6pm.h>
#include <Arduino.h>

// DRV8711 Pins
constexpr uint8_t CS1     = PA_2;
constexpr uint8_t STEP1   = PA_6;
constexpr uint8_t SLEEPn1 = PE_5;
constexpr uint8_t DIR1    = PA_7;
constexpr uint8_t RST1    = PA_5;
constexpr uint8_t STALL1  = PB_2;
constexpr uint8_t FAULT1  = PE_0;

//Half a period of the STEP signal.
//Controls rotation speed of motor.
constexpr uint16_t uSec1   = 20;

class Rotary
{
    public:
        Rotary(SPIClass spiController);
        void rotate(int32_t degrees);
        void set_rpm(uint8_t rpm);
        void Init();

    private:
        SPIClass _spiController;
        float rpm = 3; // RPM of rotary set by user. For rotary we are currently using it is a max of approx. 3.1 rpm
        uint8_t gearRatio = 72; // 72:1 gear ratio
        uint8_t uStep = 32;     // 1/32 micro-stepping
        uint32_t fullRev;
        uint32_t oneDegree;
        int32_t position = 0;
        void WriteSPI_new(unsigned char dataHi, unsigned char dataLo); 

};