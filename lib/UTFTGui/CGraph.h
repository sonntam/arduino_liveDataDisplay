#ifndef CGRAPH_H
#define CGRAPH_H

#include <Arduino.h>
#include <UTFT.h>

class CGraph
{
  private:
  
    typedef struct rgbcolor_tag{
      byte r;
      byte g;
      byte b;
    } rgbcolor;
    
    rgbcolor axisColor;
    rgbcolor axisBackgroundColor;
    rgbcolor lineColor;
    rgbcolor gridColor;
    
    struct {
      int x, y;
      int w, h;
      float x0, xf;
      float y0, yf;
    } axisDimensions;
    
    int m_eraserWidth;   //!< Eraser width in pixels
    int m_oldCursorX, m_oldCursorY;
    float m_oldValX;
    
    // Variables providing max/min drawable pixel coordinates for curves
    int m_maxX, m_maxY, m_minX, m_minY;
    
    // Variables providing pixel width and height of drawable area
    int m_dX, m_dY;
    
    // Grid draw intervals, set to 0 to skip drawing
    float m_XGridInterval;
    float m_YGridInterval;
    float m_YGridStart;     // Pre-calculated first line of Y grid
    
    void drawAALine( int x1, int y1, int x2, int y2 );
    void drawAAPixel( int x1, int y1, int colorGrad );

    boolean m_drawCursor;
    uint8_t m_numDraw;
    
    UTFT *m_tft;
    
  public:
  
    CGraph(int x, int y, int w, int h, float x0, float xf, float y0, float yf, UTFT *tft);
    ~CGraph() {};
    
    void setGrid( boolean bEnable );
    void setAxisColor( byte r, byte g, byte b );
    void setBackgroundColor( byte r, byte g, byte b );
    void setLineColor( byte r, byte g, byte b );
    void setGridColor( byte r, byte g, byte b );
    void setXGridInterval( float ival );
    void setYGridInterval( float ival );
    void setEraserPixelWidth( int ival );
    void setCursor( boolean bEnable ) { this->m_drawCursor = bEnable; };
    
    void addData( float t, float val );
    
    void redrawAxis();    //!< Redraws the axes and clears the current plot curve
};

#endif
