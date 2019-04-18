#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

// LCD
// Library only supports hardware SPI at this time
// Connect SCLK to UNO Digital #13 (Hardware SPI clock)
// Connect MISO to UNO Digital #12 (Hardware SPI MISO)
// Connect MOSI to UNO Digital #11 (Hardware SPI MOSI)
#define RA8875_INT     3
#define RA8875_CS      10
#define RA8875_RESET   9

#define NUMINPUTS 4

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

void setup() {
  Serial.begin(9600);

  /* Initialize the display using 'RA8875_480x80', 'RA8875_480x128', 'RA8875_480x272' or 'RA8875_800x480' */
  if (!tft.begin(RA8875_800x480)) {
    Serial.println("LCD not found!");
    while (1);
  }

  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  tft.fillScreen(RA8875_BLACK);
  tft.setScrollWindow(0, 0, 800, 480, RA8875_SCROLL_BOTH);
  tft.fillCircle(690, 370, 100, RA8875_WHITE);
}


void loop() {
  static int Scroll=0;
  static int Dir=1;

  tft.scrollX(Scroll);
  tft.scrollY(Scroll);

  Scroll+=Dir;
  if(Scroll >= 250) {
    Dir=-1;;
  } else if(Scroll <= 0) {
    Dir=1;
  }

  delay(10);
}
