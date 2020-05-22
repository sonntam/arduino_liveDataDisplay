#ifndef CMAX31855_H
#define CMAX31855_H

#include <Arduino.h>
#include <SPI.h>

class CMAX31855
{
  private:
  
    byte               m_portCS;                                    //!< Chip select digital port
    static SPISettings m_SPISettings;
    boolean            m_ErrorState;
    unsigned long      m_t;              //!< Time of last measurement
      
  public:
  
    CMAX31855(byte portCS);
    ~CMAX31855() {};
    
    int16_t getRawValue();    //!< Read raw sensor data (14-bit)
    float   getCelsius();
};

#endif
