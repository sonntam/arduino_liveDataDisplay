#include "CMAX31855.h"
#include <stdlib.h>

// Private variables
SPISettings CMAX31855::m_SPISettings(2000000,MSBFIRST, SPI_MODE0); 

//-----------------------------------------------------------------------------------------

CMAX31855::CMAX31855(byte portCS) : m_portCS(portCS), m_ErrorState(false)
{ 
  pinMode     ( m_portCS, OUTPUT );
  digitalWrite( m_portCS, HIGH );
}

//-----------------------------------------------------------------------------------------

float CMAX31855::getCelsius(void)
{
  int16_t rawVal = getRawValue();
  float   phyVal;
  
  phyVal = rawVal * 0.25f;
  
  return phyVal;
}

//-----------------------------------------------------------------------------------------

int16_t CMAX31855::getRawValue(void)
{
  // accessor for 32 bit value from composed bytes
  union {
    uint8_t  byte[4];
    uint32_t intval;
  } buffer;
  
  uint16_t val;
  int16_t  sval;
  
  SPI.beginTransaction( m_SPISettings );
  
  // CS
  digitalWrite( m_portCS, LOW );
  
  // Wait for first sign bit
  delayMicroseconds(2);
  
  // Timestamp
  m_t = millis();
  
  // Receive data
  for( int i = 3; i >= 0; i-- ) 
  {
    buffer.byte[i] = SPI.transfer(0x00);
  }

  digitalWrite( m_portCS, HIGH );
  
  SPI.endTransaction();
  
  // Check for error
  if( buffer.intval & ((uint32_t)1 << 16) )
  {
    m_ErrorState = true;
    return -32000;
  }
  
  m_ErrorState = false;
  
  // Assemble data
  if( buffer.intval & ((uint32_t)1 << 31) ) // Check sign bit
  {
    val = 0xC000 | ((buffer.intval >> 18) & (((uint32_t)1 << 18)-1));
  } else {
    val = buffer.intval >> 18;
  }
      
  // Cast to signed type
  sval = *( (int16_t*)(&val)  );
  
  return sval;
}

//-----------------------------------------------------------------------------------------
