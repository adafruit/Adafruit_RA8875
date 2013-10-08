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
  if (!tft.begin(RA8875_480x272)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }

  Serial.println("Found RA8875");
  /*
  //RA8875_softReset();
  for (uint8_t i=0; i<100; i++) {
    Serial.print("$"); Serial.print(i, HEX);
    Serial.print(": 0x"); Serial.println(readReg(i), HEX);
  }
  */

  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  // Play with PWM
  for (uint8_t i=255; i!=0; i-=5 ) 
  {
    tft.PWM1out(i); 
    delay(1);
  }  
  for (uint8_t i=0; i!=255; i+=5 ) 
  {
    tft.PWM1out(i); 
    delay(1);
  }
  tft.PWM1out(255); 

  // Try some GFX acceleration!
  tft.drawCircle(100, 100, 50, RA8875_BLACK);
  tft.fillCircle(100, 100, 49, RA8875_GREEN);
  
  tft.drawPixel(10,10,RA8875_BLACK);
  tft.drawPixel(11,11,RA8875_BLACK);
  tft.fillRoundRect(200, 10, 200, 100, 10, RA8875_RED);
  
  tft.fillScreen(RA8875_GREEN);
  delay(100);
  tft.fillScreen(RA8875_RED);
  delay(100);
  tft.fillScreen(RA8875_GREEN);
  delay(100);
  tft.fillScreen(RA8875_RED);
  delay(100);
  tft.fillScreen(RA8875_GREEN);
  delay(100);
  tft.fillScreen(RA8875_BLACK);

  tft.drawRect(10, 10, 400, 200, RA8875_GREEN);
  tft.fillRect(11, 11, 398, 198, RA8875_BLUE);
  tft.drawLine(10, 10, 200, 100, RA8875_RED);
  
  pinMode(RA8875_INT, INPUT);
  digitalWrite(RA8875_INT, HIGH);
  
  tft.touchEnable(true);
    
  Serial.print("Status: "); Serial.println(tft.readStatus(), HEX);
  Serial.println("Waiting for touch events ...");
}

void loop() 
{
  /* Wait around for touch events */
  if (! digitalRead(RA8875_INT)) 
  {
    if (tft.touched()) 
    {
      Serial.print("Touch: "); 
      tft.touchRead(&tx, &ty);
      Serial.print(tx); Serial.print(", "); Serial.println(ty);
    } 
  }
}

