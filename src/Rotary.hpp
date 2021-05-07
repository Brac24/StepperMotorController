#include <cstdint>
#include <SPI.h>

class Rotary
{
    public:
        Rotary();
        void rotate(int16_t degrees);

    private:
        SPIClass spiController; 

};