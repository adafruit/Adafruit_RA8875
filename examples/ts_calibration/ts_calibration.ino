#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

#define RA8875_INT     3
#define RA8875_CS      10
#define RA8875_RESET   9

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
tsPoint_t       _tsLCDPoints[3]; 
tsPoint_t       _tsTSPoints[3]; 
tsMatrix_t      _tsMatrix;

/**************************************************************************/
/*!
    @brief Calculates the difference between the touch screen and the
           actual screen co-ordinates, taking into account misalignment
           and any physical offset of the touch screen.

    @note  This is based on the public domain touch screen calibration code
           written by Carlos E. Vidales (copyright (c) 2001).

           For more information, see the following app notes:

           - AN2173 - Touch Screen Control and Calibration
             Svyatoslav Paliy, Cypress Microsystems
           - Calibration in touch-screen systems
             Wendy Fang and Tony Chang,
             Analog Applications Journal, 3Q 2007 (Texas Instruments)
*/
/**************************************************************************/
int setCalibrationMatrix( tsPoint_t * displayPtr, tsPoint_t * screenPtr, tsMatrix_t * matrixPtr)
{
  int  retValue = 0;
  
  matrixPtr->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) - 
                       ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
  
  if( matrixPtr->Divider == 0 )
  {
    retValue = -1 ;
  }
  else
  {
    matrixPtr->An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) - 
                    ((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
  
    matrixPtr->Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) - 
                    ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x)) ;
  
    matrixPtr->Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y ;
  
    matrixPtr->Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) - 
                    ((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y)) ;
  
    matrixPtr->En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) - 
                    ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x)) ;
  
    matrixPtr->Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y ;

    // Persist data to EEPROM
    // eepromWriteS32(CFG_EEPROM_TOUCHSCREEN_CAL_AN, matrixPtr->An);
    // eepromWriteS32(CFG_EEPROM_TOUCHSCREEN_CAL_BN, matrixPtr->Bn);
    // eepromWriteS32(CFG_EEPROM_TOUCHSCREEN_CAL_CN, matrixPtr->Cn);
    // eepromWriteS32(CFG_EEPROM_TOUCHSCREEN_CAL_DN, matrixPtr->Dn);
    // eepromWriteS32(CFG_EEPROM_TOUCHSCREEN_CAL_EN, matrixPtr->En);
    // eepromWriteS32(CFG_EEPROM_TOUCHSCREEN_CAL_FN, matrixPtr->Fn);
    // eepromWriteS32(CFG_EEPROM_TOUCHSCREEN_CAL_DIVIDER, matrixPtr->Divider);
    // eepromWriteU8(CFG_EEPROM_TOUCHSCREEN_CALIBRATED, 1);
  }

  return( retValue ) ;
} 

/**************************************************************************/
/*!
    @brief  Converts raw touch screen locations (screenPtr) into actual
            pixel locations on the display (displayPtr) using the
            supplied matrix.
            
    @param[out] displayPtr  Pointer to the tsPoint_t object that will hold
                            the compensated pixel location on the display
    @param[in]  screenPtr   Pointer to the tsPoint_t object that contains the
                            raw touch screen co-ordinates (before the
                            calibration calculations are made)
    @param[in]  matrixPtr   Pointer to the calibration matrix coefficients
                            used during the calibration process (calculated
                            via the tsCalibrate() helper function)

    @note  This is based on the public domain touch screen calibration code
           written by Carlos E. Vidales (copyright (c) 2001).
*/
/**************************************************************************/
int calibrateTSPoint( tsPoint_t * displayPtr, tsPoint_t * screenPtr, tsMatrix_t * matrixPtr )
{
  int  retValue = 0 ;
  
  if( matrixPtr->Divider != 0 )
  {
    displayPtr->x = ( (matrixPtr->An * screenPtr->x) + 
                      (matrixPtr->Bn * screenPtr->y) + 
                       matrixPtr->Cn 
                    ) / matrixPtr->Divider ;

    displayPtr->y = ( (matrixPtr->Dn * screenPtr->x) + 
                      (matrixPtr->En * screenPtr->y) + 
                       matrixPtr->Fn 
                    ) / matrixPtr->Divider ;
  }
  else
  {
    return -1;
  }

  return( retValue );
}

/**************************************************************************/
/*!
    @brief  Waits for a touch event
*/
/**************************************************************************/
void waitForTouchEvent(tsPoint_t * point)
{
  /* Clear the touch data object and placeholder variables */
  memset(point, 0, sizeof(tsPoint_t));
  
  /* Clear any previous interrupts to avoid false buffered reads */
  uint16_t x, y;
  tft.touchRead(&x, &y);
  delay(1);

  /* Wait around for a new touch event (INT pin goes low) */
  while (digitalRead(RA8875_INT))
  {
  }
  
  /* Make sure this is really a touch event */
  if (tft.touched())
  {
    tft.touchRead(&x, &y);
    point->x = x;
    point->y = y;
    Serial.print("Touch: ");
    Serial.print(point->x); Serial.print(", "); Serial.println(point->y);
  }
  else
  {
    point->x = 0;
    point->y = 0;
  }
}

/**************************************************************************/
/*!
    @brief  Renders the calibration screen with an appropriately
            placed test point and waits for a touch event
*/
/**************************************************************************/
tsPoint_t renderCalibrationScreen(uint16_t x, uint16_t y, uint16_t radius)
{
  tft.fillScreen(RA8875_WHITE);
  tft.drawCircle(x, y, radius, RA8875_RED);
  tft.drawCircle(x, y, radius + 2, 0x8410);  /* 50% Gray */

  // Wait for a valid touch events
  tsPoint_t point = { 0, 0 };
  
  /* Keep polling until the TS event flag is valid */
  bool valid = false;
  while (!valid)
  {
    waitForTouchEvent(&point);
    if (point.x || point.y) 
    {
      valid = true;
    }
  }
  
  return point;
}

/**************************************************************************/
/*!
    @brief  Starts the screen calibration process.  Each corner will be
            tested, meaning that each boundary (top, left, right and 
            bottom) will be tested twice and the readings averaged.
*/
/**************************************************************************/
void tsCalibrate(void)
{
  tsPoint_t data;

  /* --------------- Welcome Screen --------------- */
  Serial.println("Starting the calibration process");
  data = renderCalibrationScreen(tft.width() / 2, tft.height() / 2, 5);
  delay(250);

  /* ----------------- First Dot ------------------ */
  // 10% over and 10% down
  data = renderCalibrationScreen(tft.width() / 10, tft.height() / 10, 5);
  _tsLCDPoints[0].x = tft.width() / 10;
  _tsLCDPoints[0].y = tft.height() / 10;
  _tsTSPoints[0].x = data.x;
  _tsTSPoints[0].y = data.y;
  Serial.print("Point 1 - LCD");
  Serial.print(" X: ");
  Serial.print(_tsLCDPoints[0].x);
  Serial.print(" Y: ");
  Serial.print(_tsLCDPoints[0].y); 
  Serial.print(" TS X: ");
  Serial.print(_tsTSPoints[0].x); 
  Serial.print(" Y: ");
  Serial.println(_tsTSPoints[0].y); 
  delay(250);

  /* ---------------- Second Dot ------------------ */
  // 50% over and 90% down
  data = renderCalibrationScreen(tft.width() / 2, tft.height() - tft.height() / 10, 5);
  _tsLCDPoints[1].x = tft.width() / 2;
  _tsLCDPoints[1].y = tft.height() - tft.height() / 10;
  _tsTSPoints[1].x = data.x;
  _tsTSPoints[1].y = data.y;
  Serial.print("Point 2 - LCD");
  Serial.print(" X: ");
  Serial.print(_tsLCDPoints[1].x);
  Serial.print(" Y: ");
  Serial.print(_tsLCDPoints[1].y);
  Serial.print(" TS X: ");
  Serial.print(_tsTSPoints[1].x);
  Serial.print(" Y: ");
  Serial.println(_tsTSPoints[1].y);
  delay(250);

  /* ---------------- Third Dot ------------------- */
  // 90% over and 50% down
  data = renderCalibrationScreen(tft.width() - tft.width() / 10, tft.height() / 2, 5);
  _tsLCDPoints[2].x = tft.width() - tft.width() / 10;
  _tsLCDPoints[2].y = tft.height() / 2;
  _tsTSPoints[2].x = data.x;
  _tsTSPoints[2].y = data.y;
  Serial.print("Point 3 - LCD");
  Serial.print(" X: ");
  Serial.print(_tsLCDPoints[2].x);
  Serial.print(" Y: ");
  Serial.print(_tsLCDPoints[2].y);
  Serial.print(" TS X: ");
  Serial.print(_tsTSPoints[2].x);
  Serial.print(" Y: ");
  Serial.println(_tsTSPoints[2].y);
  delay(250);
  
  /* Clear the screen */
  tft.fillScreen(RA8875_WHITE);

  // Do matrix calculations for calibration and store to EEPROM
  setCalibrationMatrix(&_tsLCDPoints[0], &_tsTSPoints[0], &_tsMatrix);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void setup() 
{
  Serial.begin(9600);
  Serial.println("Hello, RA8875!");

  /* Initialise the display using 'RA8875_480x272' or 'RA8875_800x480' */
    if (!tft.begin(RA8875_480x272)) 
  {
    Serial.println("RA8875 not found ... check your wires!");
    while (1);
  }

  /* Enables the display and sets up the backlight */
  Serial.println("Found RA8875");
  tft.displayOn(true);
  tft.GPIOX(true); // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  /* Enable the touch screen */
  Serial.println("Enabled the touch screen");
  pinMode(RA8875_INT, INPUT);
  digitalWrite(RA8875_INT, HIGH);
  tft.touchEnable(true);

  // Try some GFX acceleration!
  //tft.drawCircle(100, 100, 50, RA8875_BLACK);
  //tft.fillCircle(100, 100, 49, RA8875_GREEN);
  //tft.drawPixel(10,10,RA8875_BLACK);
  //tft.drawPixel(11,11,RA8875_BLACK);
  //tft.drawRect(10, 10, 400, 200, RA8875_GREEN);
  //tft.fillRect(11, 11, 398, 198, RA8875_BLUE);
  //tft.drawLine(10, 10, 200, 100, RA8875_RED);
  
  tft.fillScreen(RA8875_WHITE);
  delay(100);
    
  /* Start the calibration process */
  tsCalibrate();  
  
  /* _tsMatrix should now be populated with the correct coefficients! */
  Serial.println("Waiting for touch events ...");
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void loop() 
{
  tsPoint_t raw;
  tsPoint_t calibrated;

  /* Wait around for a touch event */
  waitForTouchEvent(&raw);
  
  /* Calcuate the real X/Y position based on the calibration matrix */
  calibrateTSPoint(&calibrated, &raw, &_tsMatrix );
  
  /* Draw a single pixel at the calibrated point */
  tft.fillCircle(calibrated.x, calibrated.y, 3, RA8875_BLACK);
}

