#ifndef CPROGRESSBAR_H
#define CPROGRESSBAR_H

#include <Arduino.h>
#include <UTFT.h>

class CProgressBar
{
  private:
  
    typedef struct rgbcolor_tag{
      byte r;
      byte g;
      byte b;
    } rgbcolor;
    
    rgbcolor barColor;
    rgbcolor barAlert;
    rgbcolor backgroundColor;
    rgbcolor frameColor;

    int m_margin;
    
    float m_oldVal;
    int   m_oldXVal;
    
    float m_maxAlertVal;
    float m_minAlertVal;
    int m_maxAlertX, m_minAlertX;
    
    int   m_minX, m_maxX;
    int   m_minY, m_maxY;
    
    int   m_baseX;
    
    UTFT *m_tft;
    
    struct {
      int x, y;
      int w, h;
      float xZ;        // base Value where progressbar begins drawing (default x0)
      float x0, xf;
    } barDimensions;
    
    // TODO: further implementation/definition
    int interpolate( float val );
    
  public:
      
      CProgressBar( int x, int y, int w, int h, float x0, float xf, UTFT* tft);
      ~CProgressBar() {};
      
      void setMargin( int m );
      void setBaseValue( float xZ );
      void setMaxAlert( float xAlertMax );
      void setMinAlert( float xAlertMin );
            
      void update(float val);
      
      void redraw(void);
};

#endif
