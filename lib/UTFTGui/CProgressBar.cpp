#include "CProgressBar.h"
#include <UTFT.h>

//---------------------------------------------------------------------------------------------------

CProgressBar::CProgressBar( int x, int y, int w, int h, float x0, float xf, UTFT* tft )
{
  barDimensions.x = x;
  barDimensions.y = y;
  barDimensions.w = w;
  barDimensions.h = h;
  barDimensions.x0 = x0;
  barDimensions.xf = xf;
  
  frameColor.r = 0xFF;
  frameColor.g = 0xFF;
  frameColor.b = 0xFF;
  
  backgroundColor.r = 0;
  backgroundColor.g = 0;
  backgroundColor.b = 0;
  
  barColor.r = 0;
  barColor.g = 0xEF;
  barColor.b = 0;
  
  barAlert.r = 0xEF;
  barAlert.g = 0;
  barAlert.b = 0;
  
  m_tft = tft;
  
  setMargin(1);
  
  m_oldVal  = x0;
  m_oldXVal = m_minX-1;
  
  barDimensions.xZ = x0;
  m_baseX          = -1;
  
  setMaxAlert( xf + 1.0f );
  setMinAlert( x0 - 1.0f );
}

//---------------------------------------------------------------------------------------------------

void CProgressBar::setBaseValue( float xZ )
{
  barDimensions.xZ = xZ;
  
  m_oldVal  = xZ;
  m_baseX   = interpolate( xZ );
  
  if( m_baseX < m_minX || m_baseX > m_maxX )
  {  
    m_baseX = -1;
    m_oldXVal = m_minX-1;
  }
  else
    m_oldXVal = m_baseX;
}

//---------------------------------------------------------------------------------------------------

void CProgressBar::setMaxAlert( float xAlertMax )
{
  m_maxAlertVal = xAlertMax;
  m_maxAlertX   = interpolate(xAlertMax);
}
      
//---------------------------------------------------------------------------------------------------

void CProgressBar::setMinAlert( float xAlertMin )
{
  m_minAlertVal = xAlertMin;
  m_minAlertX   = interpolate(xAlertMin);
}

//---------------------------------------------------------------------------------------------------

int CProgressBar::interpolate( float val )
{
  return (int)((val - barDimensions.x0) / (barDimensions.xf - barDimensions.x0) * (float)(m_maxX - m_minX + 1)) + m_minX - 1;
}

//---------------------------------------------------------------------------------------------------

void CProgressBar::update(float val)
{
  int cursorX;
  
  // Calculate pixel distance (negative for decreasing values) to new value
  cursorX = interpolate(val);
  cursorX = constrain( cursorX, m_minX-1, m_maxX+1 );
  
  if( cursorX == m_oldXVal ) return; // Nothing to do
  
  if( m_baseX >= 0 )
  {
    cursorX = max( cursorX, m_minX );
    
    if( cursorX < m_oldXVal ) // Getting smaller
    {
      if( cursorX <= m_baseX )
      {
        if( m_oldXVal > m_baseX )
        {
          m_tft->setColor( backgroundColor.r, backgroundColor.g, backgroundColor.b );
          m_tft->fillRect( m_baseX+1, m_minY, m_oldXVal, m_maxY );
        }
        
        if( cursorX < m_baseX )
        {
          m_tft->setColor( barColor.r, barColor.g, barColor.b );
          m_tft->fillRect( min(m_baseX-1, m_oldXVal-1), m_minY, cursorX, m_maxY );
        }
      }
      else
      {
        m_tft->setColor( backgroundColor.r, backgroundColor.g, backgroundColor.b );
        m_tft->fillRect( cursorX+1, m_minY, m_oldXVal, m_maxY );
      }
    }
    else // cursorX > m_oldXVal
    {    
      if( cursorX >= m_baseX )
      {
        if( m_oldXVal < m_baseX )
        {
          m_tft->setColor( backgroundColor.r, backgroundColor.g, backgroundColor.b );
          m_tft->fillRect( m_baseX-1, m_minY, m_oldXVal, m_maxY );
        }
        
        if( cursorX > m_baseX )
        {
          m_tft->setColor( barColor.r, barColor.g, barColor.b );
          m_tft->fillRect( max(m_baseX+1, m_oldXVal+1), m_minY, cursorX, m_maxY );
        }
      }
      else
      {
        m_tft->setColor( backgroundColor.r, backgroundColor.g, backgroundColor.b );
        m_tft->fillRect( cursorX-1, m_minY, m_oldXVal, m_maxY );
      }
    }
  }
  else
  {
    if( cursorX < m_oldXVal ) // Getting smaller
    {
      if( m_oldXVal < m_minX ) return; // Nothing to do
      
      m_tft->setColor( backgroundColor.r, backgroundColor.g, backgroundColor.b );
      m_tft->fillRect( cursorX+1, m_minY, m_oldXVal, m_maxY );
    }
    else
    {
      // Draw new part of progressbar
      if( m_oldXVal < m_maxAlertX-1 )
      {
        m_tft->setColor( barColor.r, barColor.g, barColor.b );
        m_tft->fillRect( m_oldXVal+1, m_minY, min(cursorX, m_maxAlertX-1), m_maxY );
      }
      
      if( cursorX >= m_maxAlertX )
      {
        m_tft->setColor( barAlert.r, barAlert.g, barAlert.b );
        m_tft->fillRect( max(m_oldXVal+1,m_maxAlertX), m_minY, cursorX, m_maxY );
      }      
    }
  }
  
  m_oldVal  = val;
  m_oldXVal = cursorX;
}

//---------------------------------------------------------------------------------------------------

void CProgressBar::redraw(void)
{
  // Draw frame & margin
  m_tft->setColor( frameColor.r, frameColor.g, frameColor.b );
  m_tft->drawRect( barDimensions.x, barDimensions.y, barDimensions.x + barDimensions.w - 1, barDimensions.y + barDimensions.h - 1 );
  
  m_tft->setColor( backgroundColor.r, backgroundColor.g, backgroundColor.b );
  m_tft->fillRect( m_minX, m_minY, m_maxX, m_maxY );
  
  // Draw cursor
  if( m_baseX >= -1 )
  {
    m_tft->setColor( frameColor.r, frameColor.g, frameColor.b );
    m_tft->drawLine( m_baseX, m_minY, m_baseX, m_maxY );
  }
}

//---------------------------------------------------------------------------------------------------

void CProgressBar::setMargin( int m )
{ 
  m_margin = m; 
  
  m_minX = barDimensions.x + m + 1;
  m_maxX = barDimensions.x + barDimensions.w - 1 - m - 1;
  
  m_minY = barDimensions.y + m + 1;
  m_maxY = barDimensions.y + barDimensions.h - 1 - m - 1;
}

