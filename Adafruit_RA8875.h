/**************************************************************************/
/*!
    @file     Adafruit_RA8875.h
    @author   Limor Friend/Ladyada, K.Townsend/KTOWN for Adafruit Industries

     This is the library for the Adafruit RA8875 Driver board for TFT displays
     ---------------> http://www.adafruit.com/products/1590
     The RA8875 is a TFT driver for up to 800x480 dotclock'd displays
     It is tested to work with displays in the Adafruit shop. Other displays
     may need timing adjustments and are not guanteed to work.

     Adafruit invests time and resources providing this open
     source code, please support Adafruit and open-source hardware
     by purchasing products from Adafruit!

     Written by Limor Fried/Ladyada for Adafruit Industries.
     BSD license, check license.txt for more information.
     All text above must be included in any redistribution.
*/
/**************************************************************************/

#if ARDUINO >= 100
#include "Arduino.h"
#include "Print.h"
#else
#include "WProgram.h"
#endif

#ifdef __AVR
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif

/// @cond DISABLE
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega1280__) ||              \
    defined(__AVR_ATmega2560__) || defined(ESP8266) || defined(ESP32) ||       \
    defined(DOXYGEN)
/// @endcond
#define EEPROM_SUPPORTED ///< Board supports EEPROM Storage
/// @cond DISABLE
#endif
/// @endcond

#include <Adafruit_GFX.h>

#ifndef _ADAFRUIT_RA8875_H
#define _ADAFRUIT_RA8875_H ///< File has been included

// Touchscreen Calibration and EEPROM Storage Defines
#define CFG_EEPROM_TOUCHSCREEN_CAL_AN 0       ///< EEPROM Storage Location
#define CFG_EEPROM_TOUCHSCREEN_CAL_BN 4       ///< EEPROM Storage Location
#define CFG_EEPROM_TOUCHSCREEN_CAL_CN 8       ///< EEPROM Storage Location
#define CFG_EEPROM_TOUCHSCREEN_CAL_DN 12      ///< EEPROM Storage Location
#define CFG_EEPROM_TOUCHSCREEN_CAL_EN 16      ///< EEPROM Storage Location
#define CFG_EEPROM_TOUCHSCREEN_CAL_FN 20      ///< EEPROM Storage Location
#define CFG_EEPROM_TOUCHSCREEN_CAL_DIVIDER 24 ///< EEPROM Storage Location
#define CFG_EEPROM_TOUCHSCREEN_CALIBRATED 28  ///< EEPROM Storage Location

/// @cond DISABLE
#if defined(EEPROM_SUPPORTED)
#if defined(__AVR_ATmega328P__)
/// @endcond
#define EEPROMSIZE 1024 ///< 1KB EEPROM
/// @cond DISABLE
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
/// @endcond
#define EEPROMSIZE 4096 ///< 4KB EEPROM
/// @cond DISABLE
#else
/// @endcond
#define EEPROMSIZE 512 ///< 512 Byte EEPROM
/// @cond DISABLE
#endif
#endif
/// @endcond
// Sizes!

/**************************************************************************/
/*!
 @enum RA8875sizes The Supported Screen Sizes
 */
/**************************************************************************/
enum RA8875sizes {
  RA8875_480x80,  /*!<  480x80 Pixel Display */
  RA8875_480x128, /*!< 480x128 Pixel Display */
  RA8875_480x272, /*!< 480x272 Pixel Display */
  RA8875_800x480  /*!< 800x480 Pixel Display */
};

/**************************************************************************/
/*!
 @struct Point
 Calibration Point

 @var Point::x
    x-coordinate
 @var Point::y
    y-coordinate
 */
/**************************************************************************/
typedef struct Point {
  int32_t x;
  int32_t y;
} tsPoint_t; ///< Nameless struct variable!

/**************************************************************************/
/*!
 @struct tsMatrix_t
 Calibration Data Structure

 @var tsMatrix_t::An
 A Coefficient with the coarsest granularity
 @var tsMatrix_t::Bn
 B Coeffiecient
 @var tsMatrix_t::Cn
 C Coefficient
 @var tsMatrix_t::Dn
 D Coeffiecient
 @var tsMatrix_t::En
 E Coefficient
 @var tsMatrix_t::Fn
 F Coeffiecient with the finest granularity
 @var tsMatrix_t::Divider
 Divider for Coefficients
 */
/**************************************************************************/
typedef struct // Matrix
{
  int32_t An, Bn, Cn, Dn, En, Fn, Divider;
} tsMatrix_t;

/**************************************************************************/
/*!
 @brief  Class that stores state and functions for interacting with
 the RA8875 display controller.
 */
/**************************************************************************/
class Adafruit_RA8875 : public Adafruit_GFX {
public:
  Adafruit_RA8875(uint8_t cs, uint8_t rst);

  boolean begin(enum RA8875sizes s);
  void softReset(void);
  void displayOn(boolean on);
  void sleep(boolean sleep);

  /* Text functions */
  void textMode(void);
  void textSetCursor(uint16_t x, uint16_t y);
  void textColor(uint16_t foreColor, uint16_t bgColor);
  void textTransparent(uint16_t foreColor);
  void textEnlarge(uint8_t scale);
  void textWrite(const char *buffer, uint16_t len = 0);
  void cursorBlink(uint8_t rate);

  /* Graphics functions */
  void graphicsMode(void);
  void setXY(uint16_t x, uint16_t y);
  void pushPixels(uint32_t num, uint16_t p);
  void fillRect(void);

  /* Adafruit_GFX functions */
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void drawPixels(uint16_t *p, uint32_t count, int16_t x, int16_t y);
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

  /* HW accelerated wrapper functions (override Adafruit_GFX prototypes) */
  void fillScreen(uint16_t color);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                    int16_t y2, uint16_t color);
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                    int16_t y2, uint16_t color);
  void drawEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                   int16_t shortAxis, uint16_t color);
  void fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                   int16_t shortAxis, uint16_t color);
  void drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                 int16_t shortAxis, uint8_t curvePart, uint16_t color);
  void fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                 int16_t shortAxis, uint8_t curvePart, uint16_t color);
  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                     uint16_t color);
  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                     uint16_t color);

  /* Scroll */
  void setScrollWindow(int16_t x, int16_t y, int16_t w, int16_t h,
                       uint8_t mode);
  void scrollX(int16_t dist);
  void scrollY(int16_t dist);

  /* Backlight */
  void GPIOX(boolean on);
  void PWM1config(boolean on, uint8_t clock);
  void PWM2config(boolean on, uint8_t clock);
  void PWM1out(uint8_t p);
  void PWM2out(uint8_t p);

  /* Touch screen */
  void touchEnable(boolean on);
  boolean touched(void);
  boolean touchRead(uint16_t *x, uint16_t *y);

/// @cond DISABLE
#if defined(EEPROM_SUPPORTED)
  /// @endcond
  /* Touch screen calibration persistence*/
  uint32_t eepromReadS32(int location);
  void eepromWriteS32(int location, int32_t value);
  bool readCalibration(int location, tsMatrix_t *matrixPtr);
  void writeCalibration(int location, tsMatrix_t *matrixPtr);
/// @cond DISABLE
#endif
  /// @endcond

  /* Low level access */
  void writeReg(uint8_t reg, uint8_t val);
  uint8_t readReg(uint8_t reg);
  void writeData(uint8_t d);
  uint8_t readData(void);
  void writeCommand(uint8_t d);
  uint8_t readStatus(void);
  boolean waitPoll(uint8_t r, uint8_t f);
  uint16_t width(void);
  uint16_t height(void);
  void setRotation(int8_t rotation);
  int8_t getRotation(void);

#ifndef USE_ADAFRUIT_GFX_FONTS
  /**************************************************************************/
  /*!
     Alias of textWrite to Play nice with Arduino's Print class

     @param b The string to write

     @return The number of bytes written
   */
  /**************************************************************************/
  virtual size_t write(uint8_t b) {
    textWrite((const char *)&b, 1);
    return 1;
  }

  /**************************************************************************/
  /*!
       Alias of textWrite to Play nice with Arduino's Print class

       @param buffer The buffer to write
       @param size The size of the buffer

       @return The number of bytes written
   */
  /**************************************************************************/
  virtual size_t write(const uint8_t *buffer, size_t size) {
    textWrite((const char *)buffer, size);
    return size;
  }
#endif

private:
  void PLLinit(void);
  void initialize(void);

  /* GFX Helper Functions */
  void circleHelper(int16_t x, int16_t y, int16_t r, uint16_t color,
                    bool filled);
  void rectHelper(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color,
                  bool filled);
  void triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                      int16_t x2, int16_t y2, uint16_t color, bool filled);
  void ellipseHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                     int16_t shortAxis, uint16_t color, bool filled);
  void curveHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis,
                   int16_t shortAxis, uint8_t curvePart, uint16_t color,
                   bool filled);
  void roundRectHelper(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                       uint16_t color, bool filled);

  /* Rotation Functions */
  int16_t applyRotationX(int16_t x);
  int16_t applyRotationY(int16_t y);

  void swap(int16_t &x, int16_t &y) {
    int16_t temp = x;
    x = y;
    y = temp;
  }

  uint8_t _cs, _rst;
  uint16_t _width, _height;
  uint8_t _textScale;
  uint8_t _rotation;
  uint8_t _voffset;
  enum RA8875sizes _size;
};

// Colors (RGB565)
#define RA8875_BLACK 0x0000   ///< Black Color
#define RA8875_BLUE 0x001F    ///< Blue Color
#define RA8875_RED 0xF800     ///< Red Color
#define RA8875_GREEN 0x07E0   ///< Green Color
#define RA8875_CYAN 0x07FF    ///< Cyan Color
#define RA8875_MAGENTA 0xF81F ///< Magenta Color
#define RA8875_YELLOW 0xFFE0  ///< Yellow Color
#define RA8875_WHITE 0xFFFF   ///< White Color

// Command/Data pins for SPI
#define RA8875_DATAWRITE 0x00 ///< See datasheet
#define RA8875_DATAREAD 0x40  ///< See datasheet
#define RA8875_CMDWRITE 0x80  ///< See datasheet
#define RA8875_CMDREAD 0xC0   ///< See datasheet

// Registers & bits
#define RA8875_PWRR 0x01           ///< See datasheet
#define RA8875_PWRR_DISPON 0x80    ///< See datasheet
#define RA8875_PWRR_DISPOFF 0x00   ///< See datasheet
#define RA8875_PWRR_SLEEP 0x02     ///< See datasheet
#define RA8875_PWRR_NORMAL 0x00    ///< See datasheet
#define RA8875_PWRR_SOFTRESET 0x01 ///< See datasheet

#define RA8875_MRWC 0x02 ///< See datasheet

#define RA8875_GPIOX 0xC7 ///< See datasheet

#define RA8875_PLLC1 0x88         ///< See datasheet
#define RA8875_PLLC1_PLLDIV2 0x80 ///< See datasheet
#define RA8875_PLLC1_PLLDIV1 0x00 ///< See datasheet

#define RA8875_PLLC2 0x89        ///< See datasheet
#define RA8875_PLLC2_DIV1 0x00   ///< See datasheet
#define RA8875_PLLC2_DIV2 0x01   ///< See datasheet
#define RA8875_PLLC2_DIV4 0x02   ///< See datasheet
#define RA8875_PLLC2_DIV8 0x03   ///< See datasheet
#define RA8875_PLLC2_DIV16 0x04  ///< See datasheet
#define RA8875_PLLC2_DIV32 0x05  ///< See datasheet
#define RA8875_PLLC2_DIV64 0x06  ///< See datasheet
#define RA8875_PLLC2_DIV128 0x07 ///< See datasheet

#define RA8875_SYSR 0x10       ///< See datasheet
#define RA8875_SYSR_8BPP 0x00  ///< See datasheet
#define RA8875_SYSR_16BPP 0x0C ///< See datasheet
#define RA8875_SYSR_MCU8 0x00  ///< See datasheet
#define RA8875_SYSR_MCU16 0x03 ///< See datasheet

#define RA8875_PCSR 0x04       ///< See datasheet
#define RA8875_PCSR_PDATR 0x00 ///< See datasheet
#define RA8875_PCSR_PDATL 0x80 ///< See datasheet
#define RA8875_PCSR_CLK 0x00   ///< See datasheet
#define RA8875_PCSR_2CLK 0x01  ///< See datasheet
#define RA8875_PCSR_4CLK 0x02  ///< See datasheet
#define RA8875_PCSR_8CLK 0x03  ///< See datasheet

#define RA8875_HDWR 0x14 ///< See datasheet

#define RA8875_HNDFTR 0x15         ///< See datasheet
#define RA8875_HNDFTR_DE_HIGH 0x00 ///< See datasheet
#define RA8875_HNDFTR_DE_LOW 0x80  ///< See datasheet

#define RA8875_HNDR 0x16      ///< See datasheet
#define RA8875_HSTR 0x17      ///< See datasheet
#define RA8875_HPWR 0x18      ///< See datasheet
#define RA8875_HPWR_LOW 0x00  ///< See datasheet
#define RA8875_HPWR_HIGH 0x80 ///< See datasheet

#define RA8875_VDHR0 0x19     ///< See datasheet
#define RA8875_VDHR1 0x1A     ///< See datasheet
#define RA8875_VNDR0 0x1B     ///< See datasheet
#define RA8875_VNDR1 0x1C     ///< See datasheet
#define RA8875_VSTR0 0x1D     ///< See datasheet
#define RA8875_VSTR1 0x1E     ///< See datasheet
#define RA8875_VPWR 0x1F      ///< See datasheet
#define RA8875_VPWR_LOW 0x00  ///< See datasheet
#define RA8875_VPWR_HIGH 0x80 ///< See datasheet

#define RA8875_HSAW0 0x30 ///< See datasheet
#define RA8875_HSAW1 0x31 ///< See datasheet
#define RA8875_VSAW0 0x32 ///< See datasheet
#define RA8875_VSAW1 0x33 ///< See datasheet

#define RA8875_HEAW0 0x34 ///< See datasheet
#define RA8875_HEAW1 0x35 ///< See datasheet
#define RA8875_VEAW0 0x36 ///< See datasheet
#define RA8875_VEAW1 0x37 ///< See datasheet

#define RA8875_MCLR 0x8E            ///< See datasheet
#define RA8875_MCLR_START 0x80      ///< See datasheet
#define RA8875_MCLR_STOP 0x00       ///< See datasheet
#define RA8875_MCLR_READSTATUS 0x80 ///< See datasheet
#define RA8875_MCLR_FULL 0x00       ///< See datasheet
#define RA8875_MCLR_ACTIVE 0x40     ///< See datasheet

#define RA8875_DCR 0x90                   ///< See datasheet
#define RA8875_DCR_LINESQUTRI_START 0x80  ///< See datasheet
#define RA8875_DCR_LINESQUTRI_STOP 0x00   ///< See datasheet
#define RA8875_DCR_LINESQUTRI_STATUS 0x80 ///< See datasheet
#define RA8875_DCR_CIRCLE_START 0x40      ///< See datasheet
#define RA8875_DCR_CIRCLE_STATUS 0x40     ///< See datasheet
#define RA8875_DCR_CIRCLE_STOP 0x00       ///< See datasheet
#define RA8875_DCR_FILL 0x20              ///< See datasheet
#define RA8875_DCR_NOFILL 0x00            ///< See datasheet
#define RA8875_DCR_DRAWLINE 0x00          ///< See datasheet
#define RA8875_DCR_DRAWTRIANGLE 0x01      ///< See datasheet
#define RA8875_DCR_DRAWSQUARE 0x10        ///< See datasheet

#define RA8875_ELLIPSE 0xA0        ///< See datasheet
#define RA8875_ELLIPSE_STATUS 0x80 ///< See datasheet

#define RA8875_MWCR0 0x40         ///< See datasheet
#define RA8875_MWCR0_GFXMODE 0x00 ///< See datasheet
#define RA8875_MWCR0_TXTMODE 0x80 ///< See datasheet
#define RA8875_MWCR0_CURSOR 0x40  ///< See datasheet
#define RA8875_MWCR0_BLINK 0x20   ///< See datasheet

#define RA8875_MWCR0_DIRMASK 0x0C ///< Bitmask for Write Direction
#define RA8875_MWCR0_LRTD 0x00    ///< Left->Right then Top->Down
#define RA8875_MWCR0_RLTD 0x04    ///< Right->Left then Top->Down
#define RA8875_MWCR0_TDLR 0x08    ///< Top->Down then Left->Right
#define RA8875_MWCR0_DTLR 0x0C    ///< Down->Top then Left->Right

#define RA8875_BTCR 0x44  ///< See datasheet
#define RA8875_CURH0 0x46 ///< See datasheet
#define RA8875_CURH1 0x47 ///< See datasheet
#define RA8875_CURV0 0x48 ///< See datasheet
#define RA8875_CURV1 0x49 ///< See datasheet

#define RA8875_P1CR 0x8A         ///< See datasheet
#define RA8875_P1CR_ENABLE 0x80  ///< See datasheet
#define RA8875_P1CR_DISABLE 0x00 ///< See datasheet
#define RA8875_P1CR_CLKOUT 0x10  ///< See datasheet
#define RA8875_P1CR_PWMOUT 0x00  ///< See datasheet

#define RA8875_P1DCR 0x8B ///< See datasheet

#define RA8875_P2CR 0x8C         ///< See datasheet
#define RA8875_P2CR_ENABLE 0x80  ///< See datasheet
#define RA8875_P2CR_DISABLE 0x00 ///< See datasheet
#define RA8875_P2CR_CLKOUT 0x10  ///< See datasheet
#define RA8875_P2CR_PWMOUT 0x00  ///< See datasheet

#define RA8875_P2DCR 0x8D ///< See datasheet

#define RA8875_PWM_CLK_DIV1 0x00     ///< See datasheet
#define RA8875_PWM_CLK_DIV2 0x01     ///< See datasheet
#define RA8875_PWM_CLK_DIV4 0x02     ///< See datasheet
#define RA8875_PWM_CLK_DIV8 0x03     ///< See datasheet
#define RA8875_PWM_CLK_DIV16 0x04    ///< See datasheet
#define RA8875_PWM_CLK_DIV32 0x05    ///< See datasheet
#define RA8875_PWM_CLK_DIV64 0x06    ///< See datasheet
#define RA8875_PWM_CLK_DIV128 0x07   ///< See datasheet
#define RA8875_PWM_CLK_DIV256 0x08   ///< See datasheet
#define RA8875_PWM_CLK_DIV512 0x09   ///< See datasheet
#define RA8875_PWM_CLK_DIV1024 0x0A  ///< See datasheet
#define RA8875_PWM_CLK_DIV2048 0x0B  ///< See datasheet
#define RA8875_PWM_CLK_DIV4096 0x0C  ///< See datasheet
#define RA8875_PWM_CLK_DIV8192 0x0D  ///< See datasheet
#define RA8875_PWM_CLK_DIV16384 0x0E ///< See datasheet
#define RA8875_PWM_CLK_DIV32768 0x0F ///< See datasheet

#define RA8875_TPCR0 0x70               ///< See datasheet
#define RA8875_TPCR0_ENABLE 0x80        ///< See datasheet
#define RA8875_TPCR0_DISABLE 0x00       ///< See datasheet
#define RA8875_TPCR0_WAIT_512CLK 0x00   ///< See datasheet
#define RA8875_TPCR0_WAIT_1024CLK 0x10  ///< See datasheet
#define RA8875_TPCR0_WAIT_2048CLK 0x20  ///< See datasheet
#define RA8875_TPCR0_WAIT_4096CLK 0x30  ///< See datasheet
#define RA8875_TPCR0_WAIT_8192CLK 0x40  ///< See datasheet
#define RA8875_TPCR0_WAIT_16384CLK 0x50 ///< See datasheet
#define RA8875_TPCR0_WAIT_32768CLK 0x60 ///< See datasheet
#define RA8875_TPCR0_WAIT_65536CLK 0x70 ///< See datasheet
#define RA8875_TPCR0_WAKEENABLE 0x08    ///< See datasheet
#define RA8875_TPCR0_WAKEDISABLE 0x00   ///< See datasheet
#define RA8875_TPCR0_ADCCLK_DIV1 0x00   ///< See datasheet
#define RA8875_TPCR0_ADCCLK_DIV2 0x01   ///< See datasheet
#define RA8875_TPCR0_ADCCLK_DIV4 0x02   ///< See datasheet
#define RA8875_TPCR0_ADCCLK_DIV8 0x03   ///< See datasheet
#define RA8875_TPCR0_ADCCLK_DIV16 0x04  ///< See datasheet
#define RA8875_TPCR0_ADCCLK_DIV32 0x05  ///< See datasheet
#define RA8875_TPCR0_ADCCLK_DIV64 0x06  ///< See datasheet
#define RA8875_TPCR0_ADCCLK_DIV128 0x07 ///< See datasheet

#define RA8875_TPCR1 0x71            ///< See datasheet
#define RA8875_TPCR1_AUTO 0x00       ///< See datasheet
#define RA8875_TPCR1_MANUAL 0x40     ///< See datasheet
#define RA8875_TPCR1_VREFINT 0x00    ///< See datasheet
#define RA8875_TPCR1_VREFEXT 0x20    ///< See datasheet
#define RA8875_TPCR1_DEBOUNCE 0x04   ///< See datasheet
#define RA8875_TPCR1_NODEBOUNCE 0x00 ///< See datasheet
#define RA8875_TPCR1_IDLE 0x00       ///< See datasheet
#define RA8875_TPCR1_WAIT 0x01       ///< See datasheet
#define RA8875_TPCR1_LATCHX 0x02     ///< See datasheet
#define RA8875_TPCR1_LATCHY 0x03     ///< See datasheet

#define RA8875_TPXH 0x72  ///< See datasheet
#define RA8875_TPYH 0x73  ///< See datasheet
#define RA8875_TPXYL 0x74 ///< See datasheet

#define RA8875_INTC1 0xF0     ///< See datasheet
#define RA8875_INTC1_KEY 0x10 ///< See datasheet
#define RA8875_INTC1_DMA 0x08 ///< See datasheet
#define RA8875_INTC1_TP 0x04  ///< See datasheet
#define RA8875_INTC1_BTE 0x02 ///< See datasheet

#define RA8875_INTC2 0xF1     ///< See datasheet
#define RA8875_INTC2_KEY 0x10 ///< See datasheet
#define RA8875_INTC2_DMA 0x08 ///< See datasheet
#define RA8875_INTC2_TP 0x04  ///< See datasheet
#define RA8875_INTC2_BTE 0x02 ///< See datasheet

#define RA8875_SCROLL_BOTH 0x00   ///< See datasheet
#define RA8875_SCROLL_LAYER1 0x40 ///< See datasheet
#define RA8875_SCROLL_LAYER2 0x80 ///< See datasheet
#define RA8875_SCROLL_BUFFER 0xC0 ///< See datasheet

#endif
