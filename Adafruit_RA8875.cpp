#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_RA8875.h"

Adafruit_RA8875::Adafruit_RA8875(uint8_t CS, uint8_t RST) : Adafruit_GFX(480, 272) {
  _cs = CS;
  _rst = RST;
}

boolean Adafruit_RA8875::begin(enum RA8875sizes s) {
  _size = s;

  if (_size == RA8875_480x272) {
    _width = 480;
    _height = 272;
  } 
  if (_size == RA8875_800x480) {
    _width = 800;
    _height = 480;
  }

  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
  pinMode(_rst, OUTPUT); 
  digitalWrite(_rst, LOW);

  digitalWrite(_rst, LOW);
  delay(100);
  digitalWrite(_rst, HIGH);
  delay(100);
  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  SPI.setDataMode(SPI_MODE0);
  
  if (readReg(0) != 0x75) {
    return false;
  }

  initialize();

  SPI.setClockDivider(SPI_CLOCK_DIV4);

  return true;
}


/************************* Initialization *********************************/

void Adafruit_RA8875::softReset(void) {
  writeCommand(RA8875_PWRR);
  writeData(RA8875_PWRR_SOFTRESET);
  writeData(RA8875_PWRR_NORMAL);
  delay(1);
}

void Adafruit_RA8875::PLLinit(void) {
  if (_size == RA8875_480x272) {
    writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10);
    delay(1);
    writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
    delay(1);
  }
  if (_size == RA8875_800x480) {
    writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10);
    delay(1);
    writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
    delay(1);
  }
}

void Adafruit_RA8875::initialize(void) {
  PLLinit();
  writeReg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);

  /* Timing values */
  uint8_t pixclk;
  uint8_t hsync_start;
  uint8_t hsync_pw;
  uint8_t hsync_finetune;
  uint8_t hsync_nondisp;
  uint8_t vsync_pw; 
  uint16_t vsync_nondisp;
  uint16_t vsync_start;

  /* Set the correct values for the display being used */  
  if (_size == RA8875_480x272) 
  {
    pixclk          = RA8875_PCSR_PDATL | RA8875_PCSR_8CLK;
    hsync_nondisp   = 10;
    hsync_start     = 8;
    hsync_pw        = 48;
    hsync_finetune  = 0;
    vsync_nondisp   = 3;
    vsync_start     = 8;
    vsync_pw        = 10;
  } 
  else if (_size == RA8875_800x480) 
  {
    pixclk          = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
    hsync_nondisp   = 26;
    hsync_start     = 32;
    hsync_pw        = 96;
    hsync_finetune  = 0;
    vsync_nondisp   = 32;
    vsync_start     = 23;
    vsync_pw        = 2;
  }

  writeReg(RA8875_PCSR, pixclk);
  delay(1);
  
  /* Horizontal settings registers */
  writeReg(RA8875_HDWR, (_width / 8) - 1);                          // H width: (HDWR + 1) * 8 = 480
  writeReg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
  writeReg(RA8875_HNDR, (hsync_nondisp - hsync_finetune - 2)/8);    // H non-display: HNDR * 8 + HNDFTR + 2 = 10
  writeReg(RA8875_HSTR, hsync_start/8 - 1);                         // Hsync start: (HSTR + 1)*8 
  writeReg(RA8875_HPWR, RA8875_HPWR_LOW + (hsync_pw/8 - 1));        // HSync pulse width = (HPWR+1) * 8
  
  /* Vertical settings registers */
  writeReg(RA8875_VDHR0, (uint16_t)(_height - 1) & 0xFF);
  writeReg(RA8875_VDHR1, (uint16_t)(_height - 1) >> 8);
  writeReg(RA8875_VNDR0, vsync_nondisp-1);                          // V non-display period = VNDR + 1
  writeReg(RA8875_VNDR1, vsync_nondisp >> 8);
  writeReg(RA8875_VSTR0, vsync_start-1);                            // Vsync start position = VSTR + 1
  writeReg(RA8875_VSTR1, vsync_start >> 8);
  writeReg(RA8875_VPWR, RA8875_VPWR_LOW + vsync_pw - 1);            // Vsync pulse width = VPWR + 1
  
  /* Set active window X */
  writeReg(RA8875_HSAW0, 0);                                        // horizontal start point
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, (uint16_t)(_width - 1) & 0xFF);            // horizontal end point
  writeReg(RA8875_HEAW1, (uint16_t)(_width - 1) >> 8);
  
  /* Set active window Y */
  writeReg(RA8875_VSAW0, 0);                                        // vertical start point
  writeReg(RA8875_VSAW1, 0);  
  writeReg(RA8875_VEAW0, (uint16_t)(_height - 1) & 0xFF);           // horizontal end point
  writeReg(RA8875_VEAW1, (uint16_t)(_height - 1) >> 8);
  
  /* ToDo: Setup touch panel? */
  
  /* Clear the entire window */
  writeReg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
  delay(500); 
}

uint16_t Adafruit_RA8875::width(void) { return _width; }
uint16_t Adafruit_RA8875::height(void) { return _height; }

/************************* Graphics ***********************************/

void Adafruit_RA8875::pushPixels(uint32_t num, uint16_t p) {
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_DATAWRITE);
  while (num--) {
    SPI.transfer(p >> 8);
    SPI.transfer(p);
  }
  digitalWrite(_cs, HIGH);
}

void Adafruit_RA8875::graphicsMode(void) {
  writeCommand(RA8875_MWCR0);
  uint8_t temp = readData();
  temp &= ~RA8875_MWCR0_TXTMODE; // bit #7
  writeData(temp);
}

void Adafruit_RA8875::setXY(uint16_t x, uint16_t y) {
  writeReg(RA8875_CURH0, x);
  writeReg(RA8875_CURH1, x >> 8);
  writeReg(RA8875_CURV0, y);
  writeReg(RA8875_CURV1, y >> 8);  
}

void Adafruit_RA8875::fillRect(void) {
  writeCommand(RA8875_DCR);
  writeData(RA8875_DCR_LINESQUTRI_STOP | RA8875_DCR_DRAWSQUARE);
  writeData(RA8875_DCR_LINESQUTRI_START | RA8875_DCR_FILL | RA8875_DCR_DRAWSQUARE);
}

void Adafruit_RA8875::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  writeReg(RA8875_CURH0, x);
  writeReg(RA8875_CURH1, x >> 8);
  writeReg(RA8875_CURV0, y);
  writeReg(RA8875_CURV1, y >> 8);  
  writeCommand(RA8875_MRWC);
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_DATAWRITE);
  SPI.transfer(color >> 8);
  SPI.transfer(color);
  digitalWrite(_cs, HIGH);
}

void Adafruit_RA8875::circleHelper(int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled)
{
  /* Set X */
  writeCommand(0x99);
  writeData(x0);
  writeCommand(0x9a);
  writeData(x0 >> 8);
  
  /* Set Y */
  writeCommand(0x9b);
  writeData(y0); 
  writeCommand(0x9c);	   
  writeData(y0 >> 8);
  
  /* Set Radius */
  writeCommand(0x9d);
  writeData(r);  
  
  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));
  
  /* Draw! */
  writeCommand(RA8875_DCR);
  if (filled)
  {
    writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL);
  }
  else
  {
    writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);
  }
  
  /* Wait for the command to finish */
  bool finished = false;
  while (!finished)
  {
    uint8_t temp = readReg(RA8875_DCR);
    if (!(temp & RA8875_DCR_CIRCLE_STATUS))
      finished = true;
  }
}

void Adafruit_RA8875::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  circleHelper(x0, y0, r, color, false);
}

void Adafruit_RA8875::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  circleHelper(x0, y0, r, color, true);
}

/************************* Mid Level ***********************************/

void Adafruit_RA8875::GPIOX(boolean on) {
  if (on)
    writeReg(RA8875_GPIOX, 1);
  else 
    writeReg(RA8875_GPIOX, 0);
}

void Adafruit_RA8875::PWM1out(uint8_t p) {
  writeReg(RA8875_P1DCR, p);
}

void Adafruit_RA8875::PWM2out(uint8_t p) {
  writeReg(RA8875_P2DCR, p);
}

void Adafruit_RA8875::PWM1config(boolean on, uint8_t clock) {
  if (on) {
    writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE | (clock & 0xF));
  } else {
    writeReg(RA8875_P1CR, RA8875_P1CR_DISABLE | (clock & 0xF));
  }
}

void Adafruit_RA8875::PWM2config(boolean on, uint8_t clock) {
  if (on) {
    writeReg(RA8875_P2CR, RA8875_P2CR_ENABLE | (clock & 0xF));
  } else {
    writeReg(RA8875_P2CR, RA8875_P2CR_DISABLE | (clock & 0xF));
  }
}

void Adafruit_RA8875::touchEnable(boolean on) 
{
  if (on) 
  {
    /* Enable Touch Panel (Reg 0x70) */
    writeReg(RA8875_TPCR0, RA8875_TPCR0_ENABLE        | 
                           RA8875_TPCR0_WAIT_4096CLK  |
                           RA8875_TPCR0_WAKEDISABLE   | 
                           RA8875_TPCR0_ADCCLK_DIV4); // 10mhz max!
    /* Set Auto Mode      (Reg 0x71) */
    writeReg(RA8875_TPCR1, RA8875_TPCR1_AUTO    | 
                           RA8875_TPCR1_VREFEXT | 
                           RA8875_TPCR1_DEBOUNCE);
    /* Enable TP INT */
    writeReg(RA8875_INTC1, readReg(RA8875_INTC1) | RA8875_INTC1_TP);
  } 
  else
  {
    /* Disable TP INT */
    writeReg(RA8875_INTC1, readReg(RA8875_INTC1) & ~RA8875_INTC1_TP);
    /* Disable Touch Panel (Reg 0x70) */
    writeReg(RA8875_TPCR0, RA8875_TPCR0_DISABLE);
  }
}

boolean Adafruit_RA8875::touched(void) 
{
  if (readReg(RA8875_INTC2) & RA8875_INTC2_TP) return true;
  return false;
}

boolean Adafruit_RA8875::touchRead(uint16_t *x, uint16_t *y) 
{
  uint16_t tx, ty;
  uint8_t temp;
  
  tx = readReg(RA8875_TPXH);
  ty = readReg(RA8875_TPYH);
  temp = readReg(RA8875_TPXYL);
  tx <<= 2;
  ty <<= 2;
  tx |= temp & 0x03;        // get the bottom x bits
  ty |= (temp >> 2) & 0x03; // get the bottom y bits

  *x = tx;
  *y = ty;

  /* Clear TP INT Status */
  writeReg(RA8875_INTC2, RA8875_INTC2_TP);

  return true;
}

void Adafruit_RA8875::displayOn(boolean on) 
{
 if (on) 
   writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON);
 else
   writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPOFF);
}

void Adafruit_RA8875::sleep(boolean sleep) 
{
 if (sleep) 
   writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF | RA8875_PWRR_SLEEP);
 else
   writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF);
}

/************************* Low Level ***********************************/

void  Adafruit_RA8875::writeReg(uint8_t reg, uint8_t val) 
{
  writeCommand(reg);
  writeData(val);
}

uint8_t  Adafruit_RA8875::readReg(uint8_t reg) 
{
  writeCommand(reg);
  return readData();
}

void  Adafruit_RA8875::writeData(uint8_t d) 
{
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_DATAWRITE);
  SPI.transfer(d);
  digitalWrite(_cs, HIGH);
}

uint8_t  Adafruit_RA8875::readData(void) 
{
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_DATAREAD);
  uint8_t x = SPI.transfer(0x0);
  digitalWrite(_cs, HIGH);
  return x;
}

void  Adafruit_RA8875::writeCommand(uint8_t d) 
{
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_CMDWRITE);
  SPI.transfer(d);
  digitalWrite(_cs, HIGH);
}

uint8_t  Adafruit_RA8875::readStatus(void) 
{
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_CMDREAD);
  uint8_t x = SPI.transfer(0x0);
  digitalWrite(_cs, HIGH);
  return x;
}
