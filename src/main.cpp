/*
 * Arduino Car Display
 * ===================
 * 
 * Authors:    Harun Coskun
 *             Marcus Sonntag
 *             
 *
 * Used libraries:   UTFT
 *                   UTouch
 *                   TinyFAT
 *                   
 *
 * Hardware setup
 * --------------
 *
 * TFT-Display (digital IDE pins)
 * Pins: 38 - RS
 *       39 - WR
 *       40 - CS
 *       41 - RST
 *
 * -----------------------------------------------------------------------------------------
 *
 * SD-Card (digital IDE pins)
 * Pins: 53 - CS - Chip select
 *       51 - MOSI
 *       50 - MISO
 *       52 - CLK
 *
 * -----------------------------------------------------------------------------------------
 *
 * Touch screen
 * Pins: 6 - TCLK
 *       5 - TCS
 *       4 - TDIN
 *       3 - DOUT
 *       2 - IRQ
 *
 * -----------------------------------------------------------------------------------------
 *
 * MAX31855 has 3.3V level!
 * 6 sensors in total, each with its own CS line
 * 100ns pause after CS - low, SPI is enabled if CS is pulled low
 * Max 5 MHz clock, 14 clock cycles to read compensated thermocouple,
 * 31 clock cycles to read also the cold-reference junction temperatures (die temp)
 * Read bits on falling edge of clock
 * First bit is transmitted when CS goes to low
 *
 * Bit1 (D31) = sign bit
 * D[30:18]   = temp data MSB to LSB (2^10 to 2^-2) 2-complement (1111 1111 1111 11 = -0.25°C)
 * D16        = 0 normally, 1 on error (short to VCC/GND)
 * D15 --- ref temp
 * Spec: Resolution 0.25°C, 14-bit signed value
 *
 * Pins: 50 - MISO
 *       52 - SCK
 *       23 - CS#1
 *       25 - CS#2
 *       27 - CS#3
 *       29 - CS#4
 *       31 - CS#5
 *       33 - CS#6
 *
 * -----------------------------------------------------------------------------------------
 *  
 * Pressure sensor (MPX4250AP) (Analog - Spannung bis 5V)
 * 
 * 0.2V - 0kPa
 * 4.9V - 260kPa
 *
 * Pins: A0 - VOUT
 *
 * -----------------------------------------------------------------------------------------
 *
 * Lambda controller (TODO) (Analog?)
 *
 * -----------------------------------------------------------------------------------------
 *
 * Accelerator pedal potentiometer (5V)
 * 
 * Pins: A1 - VOUT
 *
 * -----------------------------------------------------------------------------------------
 *
 * Ignition sensor (12V)
 *
 * Pins: 19 - INT2
 *
 */
#include <Arduino.h>
#include <URTouch.h>
//#include <UTouchCD.h>
#include <UTFT.h>
#include <SPI.h>
#include <MsTimer2.h>
#include "CMAX31855.h"
#include "CMPX4250.h"
#include <UTFTGui.h>
//#include "CGraph.h"
//#include "CProgressBar.h"

#include "uText.h"

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t Ubuntu[];
extern uint8_t UbuntuBold[];
extern uint8_t arial_bold[];

extern uint8_t LucidaConsole10a[];

// Set the pins to the correct ones for your development shield
// ------------------------------------------------------------
// Arduino Uno / 2009:
// -------------------
// Standard Arduino Uno/2009 shield            : <display model>,A5,A4,A3,A2
// DisplayModule Arduino Uno TFT shield        : <display model>,A5,A4,A3,A2
//
// Arduino Mega:
// -------------------
// Standard Arduino Mega/Due shield            : <display model>,38,39,40,41
// CTE TFT LCD/SD Shield for Arduino Mega      : <display model>,38,39,40,41
//
// Remember to change the model parameter to suit your display module!
UTFT      myGLCD(ITDB32S,38,39,40,41);

CMPX4250  pSens(A0);
CMAX31855 TSens1(23);
CMAX31855 TSens2(25);
CMAX31855 TSens3(27);
CMAX31855 TSens4(29);
CMAX31855 TSens5(31);
CMAX31855 TSens6(33);

// CGraph::CGraph(int x, int y, int w, int h, float x0, float xf, float y0, float yf, UTFT *tft)
CGraph    TSens1Graph(50,0,  270,40,0,5.5,-1,1, &myGLCD);
CGraph    TSens2Graph(50,0,  270,40,0,5.5,-1,1, &myGLCD);
CGraph    TSens3Graph(50,40, 270,40,0,10,-1,1, &myGLCD);
CGraph    TSens4Graph(50,80, 270,40,0,2.5,-1,1, &myGLCD);
CGraph    TSens5Graph(50,120,270,40,0,25, -1,1, &myGLCD);

// CProgressBar::CProgressBar( int x, int y, int w, int h, float x0, float xf, UTFT* tft )
CProgressBar PBoost(50, 165, 320-50, 16, 0.0f, 100.0f, &myGLCD);
CProgressBar PAPP  (50, 165+19, 320-50, 16, 0.0f, 100.0f, &myGLCD);


uText     txtPlot(&myGLCD, 320, 240);

void setup()
{
  randomSeed(analogRead(0));
  
// Setup the LCD
  myGLCD.InitLCD();
  
  delay(5);
  
  myGLCD.clrScr();
  
  TSens2Graph.setLineColor(0,255,0);
  TSens3Graph.setLineColor(0,0,255);
  TSens4Graph.setLineColor(0,255,255);  

  TSens1Graph.setEraserPixelWidth(40);  
  TSens2Graph.setEraserPixelWidth(40);  
  TSens3Graph.setEraserPixelWidth(30);
  TSens5Graph.setEraserPixelWidth(25);
  
  // Only once or on display change
  TSens1Graph.redrawAxis();
  TSens2Graph.redrawAxis();  
  TSens3Graph.redrawAxis();
  TSens4Graph.redrawAxis();
  TSens5Graph.redrawAxis();
  
  PBoost.setBaseValue( 20.0f );
  PBoost.redraw();
  PAPP.setMaxAlert( 90.0f );
  PAPP.redraw();
  
  TSens1Graph.setXGridInterval( 0.5f );  
  TSens1Graph.setYGridInterval( 0.5f );
  TSens3Graph.setYGridInterval( 0.6f );
  TSens5Graph.setXGridInterval( 2.5f );

  TSens1Graph.setCursor(true);
  TSens2Graph.setCursor(true);
  TSens3Graph.setCursor(true);
  TSens4Graph.setCursor(true);
  TSens5Graph.setCursor(true);
  
  
  /*myGLCD.setFont(arial_bold);
  myGLCD.setColor( 255,255,255);
  myGLCD.print("Boost", 0, 165 );
  myGLCD.print("APP", 0, 165+19 );
  myGLCD.print("EngRPM", 0, 165+38 );
  myGLCD.print("OilPrs", 0, 165+57 );*/
  
  
  txtPlot.setFont(LucidaConsole10a);
  txtPlot.print(0, 165, "Ladedr", NULL );
  txtPlot.print(0, 165+19, "Gasped", NULL );
  txtPlot.print(0, 165+38, "EngRPMABCDEFGHIJKLMNOPQRSTUVWXYZ01234", NULL );
  txtPlot.print(0, 165+57, "OilPrsabcdefghijklmnopqrstuvwxyz56789", NULL );
  
  Serial.begin(115200);
}

void loop()
{
  static unsigned long t_last = millis();
  static unsigned long tmrTxtUpdate1 = millis(), tmrTxtUpdate2 = millis();
  unsigned long t, dt;
  float y, z;
  t  = millis();
  dt = t - t_last;
  
  if( dt == 0 )
    return;
    
  t_last = t;
  
  // Draw the data
  y = sin( ((float)t * 2.0f * (float)(M_PI)) / 1000.0f / 2.0f);
  z = sin( ((float)t * 2.0f * (float)(M_PI)) / 1000.0f / 8.0f);
  
//  Serial.println(y);
  
  TSens1Graph.addData((float)t/1000.0f, y );
  TSens2Graph.addData((float)t/1000.0f, y*y );
  TSens3Graph.addData((float)t/1000.0f, y );
  TSens4Graph.addData((float)t/1000.0f, 0.7*y );
  TSens5Graph.addData((float)t/1000.0f, 0.9f*y );
  
  PAPP.update(z*100.0f);
  PBoost.update(z*z*100.0f);
  
  // Soft timer
  if( t - tmrTxtUpdate1 > 200 )
  {
    myGLCD.setFont(SmallFont);
    myGLCD.setColor( 0, 255, 0 );
    myGLCD.printNumF( y, 2, 0, 0, ',', 5, ' ' );
    myGLCD.setColor( 0, 0, 255 );
    myGLCD.printNumF( y, 2, 0, 40, ',', 5, ' ' );
    tmrTxtUpdate1 = t;
  }
  if( t - tmrTxtUpdate2 > 555 )
  {  
    myGLCD.setFont(SmallFont);
    myGLCD.setColor( 0, 255, 255 );
    myGLCD.printNumF( y, 2, 0, 80, ',', 5, ' ' );
    myGLCD.setColor( 255, 0, 0 );
    myGLCD.printNumF( y, 2, 0, 120, ',', 5, ' ' );
    tmrTxtUpdate2 = t;
  }
  
  delay(10);
  
  return;

/*
  myGLCD.setColor(255, 0, 0);
  myGLCD.fillRect(0, 0, 319, 13);
  myGLCD.setColor(64, 64, 64);
  myGLCD.fillRect(0, 226, 319, 239);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(255, 0, 0);
  myGLCD.print("* Universal Color TFT Display Library *", CENTER, 1);
  myGLCD.setBackColor(64, 64, 64);
  myGLCD.setColor(255,255,0);
  myGLCD.print("<http://www.RinkyDinkElectronics.com/>", CENTER, 227);

  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRect(0, 14, 319, 225);

// Draw crosshairs
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.drawLine(159, 15, 159, 224);
  myGLCD.drawLine(1, 119, 318, 119);
  for (int i=9; i<310; i+=10)
    myGLCD.drawLine(i, 117, i, 121);
  for (int i=19; i<220; i+=10)
    myGLCD.drawLine(157, i, 161, i);

// Draw sin-, cos- and tan-lines  
  myGLCD.setColor(0,255,255);
  myGLCD.print("Sin", 5, 15);
  for (int i=1; i<318; i++)
  {
    myGLCD.drawPixel(i,119+(sin(((i*1.13)*3.14)/180)*95));
  }
  
  myGLCD.setColor(255,0,0);
  myGLCD.print("Cos", 5, 27);
  for (int i=1; i<318; i++)
  {
    myGLCD.drawPixel(i,119+(cos(((i*1.13)*3.14)/180)*95));
  }

  myGLCD.setColor(255,255,0);
  myGLCD.print("Tan", 5, 39);
  for (int i=1; i<318; i++)
  {
    myGLCD.drawPixel(i,119+(tan(((i*1.13)*3.14)/180)));
  }

  delay(2000);

  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.drawLine(159, 15, 159, 224);
  myGLCD.drawLine(1, 119, 318, 119);

// Draw a moving sinewave
  x=1;
  for (int i=1; i<(318*20); i++) 
  {
    x++;
    if (x==319)
      x=1;
    if (i>319)
    {
      if ((x==159)||(buf[x-1]==119))
        myGLCD.setColor(0,0,255);
      else
        myGLCD.setColor(0,0,0);
      myGLCD.drawPixel(x,buf[x-1]);
    }
    myGLCD.setColor(0,255,255);
    y=119+(sin(((i*1.1)*3.14)/180)*(90-(i / 100)));
    myGLCD.drawPixel(x,y);
    buf[x-1]=y;
  }

  delay(2000);
  
  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);

// Draw some filled rectangles
  for (int i=1; i<6; i++)
  {
    switch (i)
    {
      case 1:
        myGLCD.setColor(255,0,255);
        break;
      case 2:
        myGLCD.setColor(255,0,0);
        break;
      case 3:
        myGLCD.setColor(0,255,0);
        break;
      case 4:
        myGLCD.setColor(0,0,255);
        break;
      case 5:
        myGLCD.setColor(255,255,0);
        break;
    }
    myGLCD.fillRect(70+(i*20), 30+(i*20), 130+(i*20), 90+(i*20));
  }

  delay(2000);
  
  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);

// Draw some filled, rounded rectangles
  for (int i=1; i<6; i++)
  {
    switch (i)
    {
      case 1:
        myGLCD.setColor(255,0,255);
        break;
      case 2:
        myGLCD.setColor(255,0,0);
        break;
      case 3:
        myGLCD.setColor(0,255,0);
        break;
      case 4:
        myGLCD.setColor(0,0,255);
        break;
      case 5:
        myGLCD.setColor(255,255,0);
        break;
    }
    myGLCD.fillRoundRect(190-(i*20), 30+(i*20), 250-(i*20), 90+(i*20));
  }
  
  delay(2000);
  
  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);

// Draw some filled circles
  for (int i=1; i<6; i++)
  {
    switch (i)
    {
      case 1:
        myGLCD.setColor(255,0,255);
        break;
      case 2:
        myGLCD.setColor(255,0,0);
        break;
      case 3:
        myGLCD.setColor(0,255,0);
        break;
      case 4:
        myGLCD.setColor(0,0,255);
        break;
      case 5:
        myGLCD.setColor(255,255,0);
        break;
    }
    myGLCD.fillCircle(100+(i*20),60+(i*20), 30);
  }
  
  delay(2000);
  
  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);

// Draw some lines in a pattern
  myGLCD.setColor (255,0,0);
  for (int i=15; i<224; i+=5)
  {
    myGLCD.drawLine(1, i, (i*1.44)-10, 224);
  }
  myGLCD.setColor (255,0,0);
  for (int i=224; i>15; i-=5)
  {
    myGLCD.drawLine(318, i, (i*1.44)-11, 15);
  }
  myGLCD.setColor (0,255,255);
  for (int i=224; i>15; i-=5)
  {
    myGLCD.drawLine(1, i, 331-(i*1.44), 15);
  }
  myGLCD.setColor (0,255,255);
  for (int i=15; i<224; i+=5)
  {
    myGLCD.drawLine(318, i, 330-(i*1.44), 224);
  }
  
  delay(2000);
  
  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);

// Draw some random circles
  for (int i=0; i<100; i++)
  {
    myGLCD.setColor(random(255), random(255), random(255));
    x=32+random(256);
    y=45+random(146);
    r=random(30);
    myGLCD.drawCircle(x, y, r);
  }

  delay(2000);
  
  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);

// Draw some random rectangles
  for (int i=0; i<100; i++)
  {
    myGLCD.setColor(random(255), random(255), random(255));
    x=2+random(316);
    y=16+random(207);
    x2=2+random(316);
    y2=16+random(207);
    myGLCD.drawRect(x, y, x2, y2);
  }

  delay(2000);
  
  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);

// Draw some random rounded rectangles
  for (int i=0; i<100; i++)
  {
    myGLCD.setColor(random(255), random(255), random(255));
    x=2+random(316);
    y=16+random(207);
    x2=2+random(316);
    y2=16+random(207);
    myGLCD.drawRoundRect(x, y, x2, y2);
  }

  delay(2000);
  
  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);

  for (int i=0; i<100; i++)
  {
    myGLCD.setColor(random(255), random(255), random(255));
    x=2+random(316);
    y=16+random(209);
    x2=2+random(316);
    y2=16+random(209);
    myGLCD.drawLine(x, y, x2, y2);
  }

  delay(2000);
  
  myGLCD.setColor(0,0,0);
  myGLCD.fillRect(1,15,318,224);

  for (int i=0; i<10000; i++)
  {
    myGLCD.setColor(random(255), random(255), random(255));
    myGLCD.drawPixel(2+random(316), 16+random(209));
  }

  delay(2000);

  myGLCD.fillScr(0, 0, 255);
  myGLCD.setColor(255, 0, 0);
  myGLCD.fillRoundRect(80, 70, 239, 169);
  
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(255, 0, 0);
  myGLCD.print("That's it!", CENTER, 93);
  myGLCD.print("Restarting in a", CENTER, 119);
  myGLCD.print("few seconds...", CENTER, 132);
  
  myGLCD.setColor(0, 255, 0);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.print("Runtime: (msecs)", CENTER, 210);
  myGLCD.printNumI(millis(), CENTER, 225);
 
  delay (10000);
  */
}