#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

#define RA8875_INT 3
#define RA8875_CS 10
#define RA8875_RESET 9

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
uint16_t tx, ty;

void setup() 
{
  Serial.begin(9600);
  Serial.println("RA8875 start");

  /* Initialise the display using 'RA8875_480x272' or 'RA8875_800x480' */
  if (!tft.begin(RA8875_800x480)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }

  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);
  tft.fillScreen(RA8875_BLACK);

  /* Switch to text mode */  
  tft.textMode();
  
  /* If necessary, enlarge the font */
  // tft.textEnlarge(0);
  
  /* Set a solid for + bg color ... */
  // tft.textColor(RA8875_WHITE, RA8875_RED);
  /* ... or a fore color plus a transparent background */
  tft.textTransparent(RA8875_WHITE);
  
  /* Set the cursor location (in pixels) */
  tft.textSetCursor(10, 10);
  
  /* Render some text! */
  uint8_t string[15] = "Hello, World! ";
  tft.textWrite(string, 15);
  tft.textWrite(string, 15);
  tft.textWrite(string, 15);
  tft.textWrite(string, 15);
  tft.textWrite(string, 15);
  tft.textWrite(string, 15);

  /* Change the cursor location and color ... */  
  tft.textSetCursor(100, 100);
  tft.textTransparent(RA8875_RED);
  /* ... and render some more text! */
  tft.textWrite(string, 15);
  tft.textWrite(string, 15);
}

void loop() 
{
}