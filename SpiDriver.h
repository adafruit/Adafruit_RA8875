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

  /**
   * Activates the driver. Similar to SPI.beginTransaction or setting clock div, etc.
   */
  void activate();

  /**
   * Deactivates the driver after activate(). Similar to SPI.endTransaction
   */
  void deactivate();

  /**
   * Starts SPI. Usually wraps SPI.begin()
   */
  void begin();

  /**
   * Ends SPI. Usually wraps SPI.end()
   */
  void end();

  /**
   * Reads 1 byte from SPI
   * @return Read data
   */
  uint8_t receive();

  /**
   * Reads 2 bytes
   * @return Bytes read
   */
  uint16_t receive16();

  /**
   * Read n bytes
   * @param buf buffer to read into
   * @param count number of bytes to read
   * @return A status code
   */
  uint8_t receive(uint8_t *buf, size_t count);

  /**
   * Sends 1 byte
   * @param data Data to send
   */
  void send(uint8_t data);

  /**
   * Sends 2 bytes of data
   * @param data The data to send
   */
  void send16(uint16_t data);

  /**
   * Send n bytes
   * @param buf The start of the buffer to send
   * @param count Number of bytes
   */
  void send(uint8_t *buf, size_t count);

  /**
   * Sets the clock speed for the SPI controller
   * @param speed Speed in Hz
   */
  inline void setClockSpeed(uint32_t speed) {
#if SPI_HAS_TRANSACTION
    _spiSettings = SPISettings(speed, MSBFIRST, SPI_MODE0);
#endif
  }

#if USE_DMA_INTERRUPT

  /**
   * Get the underlying DMA manager, if available.
   * @return The Manager
   */
  DMAManager *getDMAManager() {
    return &dmaManager;
  }

  /**
   * Sends a chain of DMA frames, if available
   * @param head The start of the chain.
   */
  void sendChain(volatile LLI *head);

  /**
   * Starts the next chain of DMA operations
   */
  void nextDMA();

#endif
private:
  SPIClass *getSpiClass();

  SPIClass *_spiClass;
  SPISettings _spiSettings;

#if USE_DMA_INTERRUPT
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
