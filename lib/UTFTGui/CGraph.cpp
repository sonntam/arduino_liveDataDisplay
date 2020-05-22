#include "CGraph.h"
#include <UTFT.h>

void CGraph::setAxisColor( byte r, byte g, byte b )
{
  this->axisColor.r = r;
  this->axisColor.g = g;
  this->axisColor.b = b;
}

//---------------------------------------------------------------------------------------------------

void CGraph::setBackgroundColor( byte r, byte g, byte b )
{
  this->axisBackgroundColor.r = r;
  this->axisBackgroundColor.g = g;
  this->axisBackgroundColor.b = b;
}

//---------------------------------------------------------------------------------------------------

void CGraph::setLineColor( byte r, byte g, byte b )
{
  this->lineColor.r = r;
  this->lineColor.g = g;
  this->lineColor.b = b;
}

//---------------------------------------------------------------------------------------------------

void CGraph::setGridColor( byte r, byte g, byte b )
{
  this->gridColor.r = r;
  this->gridColor.g = g;
  this->gridColor.b = b;
}

//---------------------------------------------------------------------------------------------------


CGraph::CGraph(int x, int y, int w, int h, float x0, float xf, float y0, float yf, UTFT *tft)
{
  this->axisDimensions.x = x;
  this->axisDimensions.y = y;
  this->axisDimensions.w = w;
  this->axisDimensions.h = h;
  this->axisDimensions.x0 = x0;
  this->axisDimensions.xf = xf;
  this->axisDimensions.y0 = y0;
  this->axisDimensions.yf = yf;

  this->m_eraserWidth = 50;  // in pixels
  this->m_oldCursorX  = -1;
  this->m_oldCursorY  = -1;

  m_maxX = x + w - 2;
  m_minX = x + 1;

  m_maxY = y + h - 2;
  m_minY = y + 1;

  m_dX = w - 3;
  m_dY = h - 3;

  // Grids
  m_XGridInterval = 0;
  m_YGridInterval = 0;
  m_oldValX       = x0;
  
  this->m_tft = tft;

  m_drawCursor = false;
  m_numDraw    = 0;

  // Default colors
  setAxisColor( 255, 255, 255 );
  setBackgroundColor( 12, 12, 12 );
  setLineColor( 255, 0, 0 );
  setGridColor( 220, 220, 220 );
}

//---------------------------------------------------------------------------------------------------

void CGraph::redrawAxis(void)
{
  // Draw background
  m_tft->setColor(axisBackgroundColor.r, axisBackgroundColor.g, axisBackgroundColor.b);
  m_tft->fillRect(axisDimensions.x + 1, axisDimensions.y + 1, axisDimensions.x + axisDimensions.w - 2, axisDimensions.y + axisDimensions.h - 2);
  // Draw axis
  m_tft->setColor(axisColor.r, axisColor.g, axisColor.b);
  m_tft->drawRect(axisDimensions.x, axisDimensions.y, axisDimensions.x + axisDimensions.w - 1, axisDimensions.y + axisDimensions.h - 1);
}

//---------------------------------------------------------------------------------------------------

void CGraph::addData( float t, float val )
{
  // Smart redraw with minimal updates

  float dx = this->axisDimensions.xf - this->axisDimensions.x0;
  int   cursorX, cursorY, cursorYC;

  // Convert to window range, e.g. x0 = 0s, xf = 5s, t = 8.7s --> becomes t = 3.7s (sort-of modulo operator)
  t -= (int)( t / dx ) * dx;

  // Calculate cursor pixel position for current time
  cursorX = min( (int)(t / dx * (float)( m_dX ) ) + m_minX, m_maxX );

  // Nothing new to plot?
  if ( cursorX == this->m_oldCursorX ) return;

  // Get new y cursor pixel value and its constrained value wrt. y-axis min/max
  cursorY  = (int)( (axisDimensions.yf - val) / (axisDimensions.yf - axisDimensions.y0) * (float)(m_dY) ) + m_minY;
  cursorYC = constrain( cursorY, m_minY, m_maxY );

  // TODO: Clear with eraser width

  // Check for any grids we need to draw up to the current cursor X value
  // Are there any grid lines between old cursor and current cursor value? if so, draw them first before
  // we generate the plot lines (otherwise the grid lines would overlay the data plot)
  if( m_XGridInterval != 0.0f ) {
    
    float gridPos = axisDimensions.x0;
    int gridCursorX, gridCursorY;

    
    while( gridPos < m_oldValX ) gridPos += m_XGridInterval;
      
    while(1)
    {
        if( gridPos > t ) break;
        
        // Draw grid at this position
        gridCursorX = min( (int)(gridPos / dx * (float)( m_dX ) ) + m_minX, m_maxX );
        
        m_tft->setColor( this->gridColor.r, this->gridColor.g, this->gridColor.b );
        
        // Grid line algorithm for dashed 2x0 2x1 line
        gridCursorY = m_minY;
        while(gridCursorY + 1 <= m_maxY) 
        {
          m_tft->drawLine( gridCursorX, gridCursorY, gridCursorX, gridCursorY + 1 );
          gridCursorY += 4;
        }
        
        // Next grid line
        gridPos += m_XGridInterval;
    }
  }
  
  // Y Grid
  if( m_YGridInterval != 0.0f ) {
    
    float gridPos = m_YGridStart;
    int gridCursorX, gridCursorY;
    int dashTemp;

    while(1)
    {
        if( gridPos > axisDimensions.yf ) break;
        
        // Draw grid at this position
        gridCursorY = (int)( (axisDimensions.yf - gridPos) / (axisDimensions.yf - axisDimensions.y0) * (float)(m_dY) ) + m_minY;
        
        m_tft->setColor( this->gridColor.r, this->gridColor.g, this->gridColor.b );
        
        // Grid line algorithm for dashed 2x0 2x1 line
        if( m_oldCursorX < 0 )
          gridCursorX = m_minX;
        else if( m_oldCursorX > cursorX )
        {
          gridCursorX = m_oldCursorX+1;
          // Draw grid lines to end of plot for a clean look and then start from the beginning to the current point
          while(gridCursorX <= m_maxX) 
          {
            dashTemp = ( gridCursorX - m_minX ) % 4;
            if( dashTemp == 0 || dashTemp == 1 )
            {
              m_tft->drawPixel( gridCursorX, gridCursorY );
            }
            gridCursorX++;
          }
          
          gridCursorX = m_minX;
        }
        else
        {
          gridCursorX = m_oldCursorX + 1;
        }
        
        // Draw the line from the last point to the current one
        while(gridCursorX <= cursorX) 
        {
          dashTemp = ( gridCursorX - m_minX ) % 4;
          if( dashTemp == 0 || dashTemp == 1 )
          {
            m_tft->drawPixel( gridCursorX, gridCursorY );
          }
          gridCursorX++;
        }
        
        // Next grid line
        gridPos += m_YGridInterval;
    }
  }

  if ( m_oldCursorX < 0 ) // We started afresh!
  {
    m_tft->setColor( this->lineColor.r, this->lineColor.g, this->lineColor.b );
    m_tft->drawPixel( cursorX, cursorYC );
    m_oldCursorX = cursorX;
    m_oldCursorY = cursorYC;
    m_oldValX    = t;
    return;
  }

  // Do we start from the beginning again? Interpolate to end of graph for clean display
  if ( cursorX < this->m_oldCursorX )
  {
    m_numDraw++;
    
    // Calculate interpolated end value
    int dxE; // Distance to end of plot of last known value
    int dxP, dyP; // Distance to new point from last known value
    int cursorYEnd;


    dxE = m_maxX - this->m_oldCursorX;
    dyP = cursorY - this->m_oldCursorY;
    dxP = cursorX - axisDimensions.x + 1 + dxE;

    cursorYEnd = constrain( this->m_oldCursorY + (int)((float)dxE / (float)dxP * dyP), axisDimensions.y + 1, axisDimensions.y + axisDimensions.h - 2 );

    // TODO draw correctly when running into saturation

    // First delete stuff at the beginning
    m_tft->setColor( axisBackgroundColor.r, axisBackgroundColor.g, axisBackgroundColor.b );
    m_tft->fillRect( m_minX, m_minY, min(cursorX + m_eraserWidth, m_maxX), m_maxY );

    m_tft->setColor( this->lineColor.r, this->lineColor.g, this->lineColor.b );
    m_tft->drawLine( this->m_oldCursorX, this->m_oldCursorY, m_maxX, cursorYEnd );
    m_tft->drawLine( m_minX, cursorYEnd, cursorX, cursorYC );
    //drawAALine( this->m_oldCursorX, this->m_oldCursorY, m_maxX, cursorYEnd );
    //drawAALine( m_minX, cursorYEnd, cursorX, cursorYC );

  } else {
    // First delete stuff at the beginning
    if ( m_oldCursorX + m_eraserWidth < m_maxX || m_drawCursor )
    {
      m_tft->setColor( axisBackgroundColor.r, axisBackgroundColor.g, axisBackgroundColor.b );
      // TODO Fix disappearing grid
      m_tft->fillRect( m_oldCursorX + m_eraserWidth, m_minY, min(cursorX + m_eraserWidth, m_maxX), m_maxY );
    }

    m_tft->setColor( this->lineColor.r, this->lineColor.g, this->lineColor.b );
    m_tft->drawLine( this->m_oldCursorX, this->m_oldCursorY, cursorX, cursorYC );
    //drawAALine( this->m_oldCursorX, this->m_oldCursorY, cursorX, cursorYC );
  }

  // Draw cursor
  if( m_drawCursor && cursorX < m_maxX ) {
    m_tft->setColor( axisColor.r, axisColor.g, axisColor.b );
    //m_tft->drawLine( cursorX+1, m_minY, cursorX+1, m_maxY );
  }

  this->m_oldCursorX = cursorX;
  this->m_oldCursorY = cursorYC;
  this->m_oldValX    = t;

}

//------------------------------------------------------------------------------------

// Draw anti-aliased line
void CGraph::drawAALine( int x1, int y1, int x2, int y2 )
{
  uint16_t IntensityShift, ErrorAdj, ErrorAcc;
  uint16_t ErrorAccTemp, Weighting, WeightingComplementMask;
  int16_t DeltaX, DeltaY, Temp, XDir;
  const int16_t BaseColor = 0;
  const int16_t numLevels = 256;
  const uint16_t intensityBits = 8;

  /* Make sure the line runs top to bottom */
  if (y1 > y2) {
    Temp = y1; y1 = y2; y2 = Temp;
    Temp = x1; x1 = x2; x2 = Temp;
  }
  /* Draw the initial pixel, which is always exactly intersected by
     the line and so needs no weighting */
  m_tft->setColor(lineColor.r, lineColor.g, lineColor.b);
  m_tft->drawPixel(x1, y1);

  if ((DeltaX = x2 - x1) >= 0) {
    XDir = 1;
  } else {
    XDir = -1;
    DeltaX = -DeltaX; /* make DeltaX positive */
  }
  /* Special-case horizontal, vertical, and diagonal lines, which
     require no weighting because they go right through the center of
     every pixel */
  if ((DeltaY = y2 - y1) == 0) {
    /* Horizontal line */
    m_tft->drawLine(x1, y1, x2, y2);
    return;
  }
  if (DeltaX == 0) {
    /* Vertical line */
    m_tft->drawLine(x1, y1, x2, y2);
    return;
  }
  if (DeltaX == DeltaY) {
    /* Diagonal line */
    m_tft->drawLine(x1, y1, x2, y2);
    return;
  }
  /* Line is not horizontal, diagonal, or vertical */
  ErrorAcc = 0;  /* initialize the line error accumulator to 0 */
  /* # of bits by which to shift ErrorAcc to get intensity level */
  IntensityShift = 16 - intensityBits;
  /* Mask used to flip all bits in an intensity weighting, producing the
     result (1 - intensity weighting) */
  WeightingComplementMask = numLevels - 1;
  /* Is this an X-major or Y-major line? */
  if (DeltaY > DeltaX) {
    /* Y-major line; calculate 16-bit fixed-point fractional part of a
       pixel that X advances each time Y advances 1 pixel, truncating the
       result so that we won't overrun the endpoint along the X axis */
    ErrorAdj = ((unsigned long) DeltaX << 16) / (unsigned long) DeltaY;
    /* Draw all pixels other than the first and last */
    while (--DeltaY) {
      ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */
      ErrorAcc += ErrorAdj;      /* calculate error for next pixel */
      if (ErrorAcc <= ErrorAccTemp) {
        /* The error accumulator turned over, so advance the X coord */
        x1 += XDir;
      }
      y1++; /* Y-major, so always advance Y */
      /* The intensityBits most significant bits of ErrorAcc give us the
         intensity weighting for this pixel, and the complement of the
         weighting for the paired pixel */
      Weighting = ErrorAcc >> IntensityShift;
      drawAAPixel(x1, y1, BaseColor + Weighting);
      drawAAPixel(x1 + XDir, y1,
                        BaseColor + (Weighting ^ WeightingComplementMask));
    }
    /* Draw the final pixel, which is
       always exactly intersected by the line
       and so needs no weighting */
    drawAAPixel(x2, y2, BaseColor);
    return;
  }
  /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
     pixel that Y advances each time X advances 1 pixel, truncating the
     result to avoid overrunning the endpoint along the X axis */
  ErrorAdj = ((unsigned long) DeltaY << 16) / (unsigned long) DeltaX;
  /* Draw all pixels other than the first and last */
  while (--DeltaX) {
    ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */
    ErrorAcc += ErrorAdj;      /* calculate error for next pixel */
    if (ErrorAcc <= ErrorAccTemp) {
      /* The error accumulator turned over, so advance the Y coord */
      y1++;
    }
    x1 += XDir; /* X-major, so always advance X */
    /* The intensityBits most significant bits of ErrorAcc give us the
       intensity weighting for this pixel, and the complement of the
       weighting for the paired pixel */
    Weighting = ErrorAcc >> IntensityShift;
    drawAAPixel(x1, y1, BaseColor + Weighting);
    drawAAPixel(x1, y1 + 1,
                      BaseColor + (Weighting ^ WeightingComplementMask));
  }
  /* Draw the final pixel, which is always exactly intersected by the line
     and so needs no weighting */
  drawAAPixel(x2, y2, BaseColor);
}

//---------------------------------------------------------------------------------------------------

void CGraph::drawAAPixel( int x1, int y1, int colorGrad )
{
  // Interpolate color
  byte r, g, b;

  r = map( colorGrad, 0, 255, lineColor.r, axisBackgroundColor.r );
  g = map( colorGrad, 0, 255, lineColor.g, axisBackgroundColor.g );
  b = map( colorGrad, 0, 255, lineColor.b, axisBackgroundColor.b );
  
  m_tft->setColor(r,g,b);
  m_tft->drawPixel( x1, y1 );
}

//---------------------------------------------------------------------------------------------------

void CGraph::setXGridInterval( float ival )
{
  m_XGridInterval = ival;
}

//---------------------------------------------------------------------------------------------------

void CGraph::setYGridInterval( float ival )
{
  m_YGridInterval = ival;
  m_YGridStart    = 0.0f;
  
  // Get lowest grid line value (searching from zero)
  while( m_YGridStart - m_YGridInterval > axisDimensions.y0 ) m_YGridStart -= m_YGridInterval;
  while( m_YGridStart < axisDimensions.y0 )                   m_YGridStart += m_YGridInterval; 
}

//---------------------------------------------------------------------------------------------------

void CGraph::setEraserPixelWidth( int ival )
{
  m_eraserWidth = ival;
}
