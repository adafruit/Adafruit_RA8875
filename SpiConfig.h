/**
 * Copyright (c) 2011-2020 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/**
 * \file
 * \brief configuration definitions. Adapted from SdFat-beta by Bill Greiman
 */
#ifndef _ADAFRUIT_RA8875_SPICONFIG_H
#define _ADAFRUIT_RA8875_SPICONFIG_H

#include <cstdint>
#include <cstddef>
#include <Arduino.h>
#include <SPI.h>

#define SPI_DRIVER 0

/**
 * Determine the default SPI configuration.
 */
#if defined(ARDUINO_ARCH_APOLLO3)\
  || (defined(__AVR__) && defined(SPDR) && defined(SPSR) && defined(SPIF))\
  || (defined(__AVR__) && defined(SPI0) && defined(SPI_RXCIF_bm))\
  || defined(ESP8266) || defined(ESP32)\
  || defined(PLATFORM_ID)\
  || defined(ARDUINO_SAM_DUE)\
  || defined(__STM32F1__) || defined(__STM32F4__)\
  || (defined(CORE_TEENSY) && defined(__arm__))
#define HAS_CUSTOM_SPI 1
#else  // HAS_CUSTOM_SPI
// Use standard SPI library.
#define HAS_CUSTOM_SPI 0
#endif  // HAS_CUSTOM_SPI

#if HAS_CUSTOM_SPI && SPI_DRIVER == 0
#define USE_CUSTOM_SPI
#endif


#endif // _ADAFRUIT_RA8875_SPICONFIG_H
