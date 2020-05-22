#ifndef CTEXTDISPLAY_H
#define CTEXTDISPLAY_H

#include <Arduino.h>

class CTextDisplay
{
  private:
  
    String m_prevStr;
    int m_x, m_y;
    
  public:
  
    CTextDisplay( int x, int y ) : m_x(x), m_y(y) {};
    ~CTextDisplay() {};
    void Update(String str);
};

#endif
