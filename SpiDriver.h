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
 * \brief SpiDriver classes for Arduino compatible systems. Adapted from SdFat-beta by Bill Greiman
 */
#ifndef _ADAFRUIT_RA8875_SPI_DRIVER_H
#define _ADAFRUIT_RA8875_SPI_DRIVER_H

#include "SpiConfig.h"
#include "DMAManager.h"

class SpiDriver {
public:
  explicit SpiDriver(uint8_t csPin, bool interrupts = false);

  void activate();

  void deactivate();

  void begin();

  void end();

  uint8_t receive();

  uint16_t receive16();

  uint8_t receive(uint8_t *buf, size_t count);

  void send(uint8_t data);

  void send16(uint16_t data);

  void send(uint8_t *buf, size_t count);

  inline void setClockSpeed(uint32_t speed) {
#if SPI_HAS_TRANSACTION
    _spiSettings = SPISettings(speed, MSBFIRST, SPI_MODE0);
#endif
  }

#ifdef USE_CUSTOM_SPI
  DMAManager* getDMAManager() {
    return &dmaManager;
  }
  void sendChain(volatile LLI* head);
  void nextDMA();
#endif
private:
  SPIClass *getSpiClass();

  SPIClass *_spiClass;
  SPISettings _spiSettings;
#ifdef USE_CUSTOM_SPI
  bool use_dma_interrupts = false;
  DMAManager dmaManager;
#endif
};

typedef SpiDriver SpiDriver;

#ifndef USE_CUSTOM_SPI
#include "SpiDefaultDriver.h"
#elif defined(__AVR__)
// Include AVR Definitions
#endif

#endif // _ADAFRUIT_RA8875_SPI_DRIVER_H
