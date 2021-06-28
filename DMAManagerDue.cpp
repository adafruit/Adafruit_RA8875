#include "SpiConfig.h"

#if USE_DMA_INTERRUPT

#include <Arduino.h>
#include "DMAManager.h"
#include "Adafruit_RA8875.h"

static void volatile_memcpy(volatile void *dst, volatile void *src, size_t size) {
  volatile auto *start = (volatile uint8_t *)src;
  volatile auto *end = (volatile uint8_t *)dst;
  for (size_t i = 0; i < size; i++) {
    *(end + i) = *(start + i);
  }
}

_LLI::_LLI() : CTRLA(0), CTRLB(0) {
  SADDR = 0;
  DADDR = 0;
  DSCR = 0;
}

_LLI::_LLI(volatile _LLI &other) : CTRLA(other.CTRLA.raw), CTRLB(other.CTRLB.raw) {
  SADDR = other.SADDR;
  DADDR = other.DADDR;
  DSCR = other.DSCR;
}

_LLI::_LLI(_LLI const volatile &&other) noexcept: CTRLA(other.CTRLA.raw), CTRLB(other.CTRLB.raw) {
  SADDR = other.SADDR;
  DADDR = other.DADDR;
  DSCR = other.DSCR;
}

_LLI::_LLI(Word SADDR, Word DADDR, Word CTRLA, Word CTRLB) : SADDR(SADDR), DADDR(DADDR), CTRLA(CTRLA), CTRLB(CTRLB) {
  DSCR = 0;
}

bool DMAManager::add_entry(volatile LLI *item) {
  return add_entry(*item);
}

bool DMAManager::add_entry(const volatile LLI &item) {
  if (size >= LLI_MAX_FRAMES) return false;
  volatile LLI *tail = get_next_blank_entry();
  tail->SADDR = item.SADDR;
  tail->DADDR = item.DADDR;
  tail->CTRLA.raw = item.CTRLA.raw;
  tail->CTRLB.raw = item.CTRLB.raw;
  size++;
  return true;
}

volatile LLI *DMAManager::get_next_blank_entry() {
  if (size >= LLI_MAX_FRAMES) return nullptr;
  return &dma_frames[size];
}

volatile LLI *DMAManager::get_entry(size_t idx, bool force) {
  if (idx >= size && !force) {
    return nullptr;
  }
  if (force && idx >= LLI_MAX_FRAMES) {
    return nullptr;
  }
  return &dma_frames[idx];
}

void DMAManager::remove_entry(uint16_t idx) {
  if (idx >= size)
    return;

  if (idx < size - 1) { // Move every entry down
    uint16_t elements_moved = size - idx - 1;
    volatile_memcpy(&dma_frames[idx], &dma_frames[idx + 1], sizeof(LLI) * elements_moved);
  }

  size--;
}

volatile LLI *DMAManager::finalize() {
  for (uint i = 0; i < size - 1; i++) {
    dma_frames[i].CTRLA.DONE = 0;
    dma_frames[i].DSCR = (uint32_t)&dma_frames[i + 1];
  }
  dma_frames[size - 1].CTRLA.DONE = 0;
  dma_frames[size - 1].DSCR = 0;
  return dma_frames;
}

void DMAManager::clear_frames() {
  size = 0;
}

void DMAManager::reset(bool full) {
  clear_frames();
  transaction_data.reset(full);
}

size_t DMAManager::get_size() const {
  return size;
}

bool DMAManager::can_add_entries(size_t entries) const {
  bool result = size + entries <= LLI_MAX_FRAMES;
  return result;
}

/*****************************
 * Frame Add Functions
 *****************************/

WoReg *DMAManager::get_pio_reg(bool state, uint8_t pin) {
  Pio *pin_descriptor = digitalPinToPort(pin);
  if (state) {
    return &pin_descriptor->PIO_SODR;
  }
  return &pin_descriptor->PIO_CODR;
}

bool DMAManager::add_entry_cs_pin_toggle(bool state, size_t num_transfers) {
  return add_entry_pin_toggle(state, _csPin, &_csPinMask, num_transfers);
}

bool DMAManager::add_entry_pin_toggle(bool state, uint8_t pin, const uint32_t *pin_mask_ptr, size_t num_transfers) {

  volatile LLI entry = {
    (Word)pin_mask_ptr,
    (Word)get_pio_reg(state, pin),
    (Word)(num_transfers | DMAC_CTRLA_SRC_WIDTH_WORD | DMAC_CTRLA_DST_WIDTH_WORD),
    (Word)(DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_FIXED),
  };

  return add_entry(entry);
}

bool DMAManager::add_entry_spi_transfer(volatile uint8_t *buf, size_t qty) {
  if (qty <= 0) return false;

  static volatile uint8_t ff = 0XFF;
  uint32_t src_incr = DMAC_CTRLB_SRC_INCR_INCREMENTING;
  if (!buf) {
    buf = &ff;
    src_incr = DMAC_CTRLB_SRC_INCR_FIXED;
  }

  volatile LLI entry = {
    (Word)buf,
    (Word)&SPI0->SPI_TDR,
    qty | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE,
    DMAC_CTRLB_FC_MEM2PER_DMA_FC | src_incr | DMAC_CTRLB_DST_INCR_FIXED
  };

  return add_entry(entry);
}

volatile CoordEntryCommand *DMAManager::add_entry_coord_bits(uint16_t coord, uint8_t coord_register) {
  if (!transaction_data.can_add_working_data(COORD_BUF_SPACE)) { // High & Low bits
    return false;
  }

  if (coord_register != RA8875_CURH0 &&
      coord_register != RA8875_CURH1 &&
      coord_register != RA8875_CURV0 &&
      coord_register != RA8875_CURV1
    )
    return false;

  uint8_t coord_commands[COORD_BUF_SPACE];

  coord_commands[0] = RA8875_CMDWRITE;
  coord_commands[1] = coord_register;
  coord_commands[2] = RA8875_DATAWRITE;
  if (coord_register == RA8875_CURH1 || coord_register == RA8875_CURV1) {
    coord_commands[3] = coord >> 8;
  } else {
    coord_commands[3] = coord & 0xFF;
  }

  volatile uint8_t *data_ptr = transaction_data.add_working_data(coord_commands, COORD_BUF_SPACE);
  if (!add_entry_spi_transfer(data_ptr, COORD_BUF_SPACE)) {
    return nullptr;
  }
  return (CoordEntryCommand *)data_ptr;
}

volatile uint8_t *DMAManager::add_entry_spi_draw_pixels(volatile uint8_t *buf, size_t qty) {
  if (!transaction_data.can_add_working_data(3)) { // High & Low bits
    return false;
  }

  if (!can_add_entries(2)) {
    return false;
  }

  uint8_t draw_commands[3] = {
    RA8875_CMDWRITE,
    RA8875_MRWC,
    RA8875_DATAWRITE
  };

  volatile uint8_t *data_ptr = transaction_data.add_working_data(draw_commands, 3);
  auto ret1 = add_entry_spi_transfer(data_ptr, 3);
  auto ret2 = add_entry_spi_transfer(buf, qty);
  if (!(ret1 && ret2)) {
    return nullptr;
  }
  return data_ptr;
}

#endif