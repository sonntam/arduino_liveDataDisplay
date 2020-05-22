#include "CMPX4250.h"

//-----------------------------------------------------------------------------------------

/*
 * Get raw ADC value and check for short to ground/5V
 * If no short is detected return raw value, otherwise return last valid measurement
 */
int CMPX4250::getRawValue() {
  int adcVal;

  adcVal = analogRead(m_APin);

  // Check for short of sensor
  if ( (adcVal > binMax || adcVal < binMin) && m_ErrorCounter < 0xFF )
  {
    m_ErrorCounter++;
    return m_lastValue;
  }
  else if ( m_ErrorCounter > 0 )
    m_ErrorCounter--;

  return (m_lastValue = adcVal);
}

//-----------------------------------------------------------------------------------------

/*
 * Get pressure value in Pa = N/m^2
 */
long CMPX4250::getPressure()
{
  int sensVal;

  sensVal = getRawValue();

  return (((long)(sensVal - binMin)) * (pMax - pMin)) / ((long)(binMax - binMin));
}

//-----------------------------------------------------------------------------------------

/*
 * Return true if error (short to ground/5V) is detected, false otherwise
 */
boolean CMPX4250::errorState()
{
  if ( this->m_ErrorCounter > 0x1F )
    return true;

  return false;
}

//-----------------------------------------------------------------------------------------
