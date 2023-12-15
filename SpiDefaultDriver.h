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
 * \brief Class using only simple SPI library functions. Adapted from SdFat-beta by Bill Greiman
 */

#ifndef _ADAFRUIT_RA8875_SPIDEFAULTDRIVER_H
#define _ADAFRUIT_RA8875_SPIDEFAULTDRIVER_H

inline SpiDriver::SpiDriver(uint8_t csPin, bool interrupts) {
  _spiClass = getSpiClass();
  setClockSpeed(2000000L);
}

inline SPIClass *SpiDriver::getSpiClass() {
  return &SPI;
}

inline void SpiDriver::activate() {
#if SPI_HAS_TRANSACTION
  _spiClass->beginTransaction(_spiSettings);
#endif // SPI_HAS_TRANSACTION
}

inline void SpiDriver::deactivate() {
#if SPI_HAS_TRANSACTION
  _spiClass->endTransaction();
#endif // SPI_HAS_TRANSACTION
}

inline void SpiDriver::begin() {
  _spiClass->begin();
}

inline void SpiDriver::end() {
  _spiClass->end();
}

inline uint8_t SpiDriver::receive() {
  return _spiClass->transfer(0xff);
}

inline uint16_t SpiDriver::receive16() {
  return _spiClass->transfer16(0xff);
}

inline uint8_t SpiDriver::receive(uint8_t *buf, size_t count) {
#ifdef __AVR__
  for (size_t i = 0; i < count; i++) {
      buf[i] = _spiClass->transfer(0xff);
  }
  return 0;
#endif
  memset(buf, 0xFF, count);
  _spiClass->transfer(buf, count);
  return 0;
}

inline void SpiDriver::send(uint8_t data) {
  _spiClass->transfer(data);
}

inline void SpiDriver::send16(uint16_t data) {
  _spiClass->transfer16(data);
}

inline void SpiDriver::send(uint8_t *buf, size_t count) {
#ifdef __AVR__
  for (size_t i = 0; i < count; i++) {
      buf[i] = _spiClass->transfer(0xff);
  }
  return;
#endif
  _spiClass->transfer(buf, count);
}

#endif // _ADAFRUIT_RA8875_SPIDEFAULTDRIVER_H
