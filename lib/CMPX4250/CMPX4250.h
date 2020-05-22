
#ifndef CMPX4250_H
#define CMPX4250_H

#include <Arduino.h>

class CMPX4250
{
  private:
  
    static const int binMin = 41;   // Binary ADC offset (see datasheet, 0.2V equals 0 kPa, 0.2V/5V*1023 = 41)
    static const int binMax = 1003; // Binary ADC offset (see datasheet, 4.9V equals 260 kPa, 4.9V/5V*1023 = 1003)
    
    static const long pMax   = 260000;  // Maximum measurable pressure / Pa
    static const long pMin   = 0;       // Minimum measurable pressure / Pa
    
    byte m_APin;              // Input ADC pin
    byte m_ErrorCounter;      // Error counter state
    int  m_lastValue;         // Last valid raw measurement value
  
  public:
     CMPX4250(byte APin) : m_APin(APin), m_ErrorCounter(0), m_lastValue(binMin) {};
    ~CMPX4250() {};
    
    /*
     * Get raw ADC value and check for short to ground/5V 
     * If no short is detected return raw value, otherwise return last valid measurement
     */
    int      getRawValue();
    
    /*
     * Get pressure value in Pa = N/m^2
     */
    long     getPressure();
    
    /*
     * Return true if error (short to ground/5V) is detected, false otherwise
     */
    boolean  errorState();
};

#endif
