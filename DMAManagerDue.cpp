#include <Arduino.h>
#include "DMAManager.h"
#include "Adafruit_RA8875.h"

/**
 * Makes no size checks, ensure space before using
 */
static void fill_command(uint8_t *buf, uint8_t keyword, uint8_t data) {
  *buf = keyword;
  *(buf + 1) = data;
}

_LLI::_LLI() {
  SADDR = 0;
  DADDR = 0;
  CTRLA = 0;
  CTRLB = 0;
  DSCR = 0;
}

_LLI::_LLI(volatile _LLI &other) {
  SADDR = other.SADDR;
  DADDR = other.DADDR;
  CTRLA = other.CTRLA;
  CTRLB = other.CTRLB;
  DSCR = other.DSCR;
}

_LLI::_LLI(_LLI const volatile &&other) {
  SADDR = other.SADDR;
  DADDR = other.DADDR;
  CTRLA = other.CTRLA;
  CTRLB = other.CTRLB;
  DSCR = other.DSCR;
}

bool DMAManager::add_entry(LLI *item) {
  return add_entry(item->SADDR, item->DADDR, item->CTRLA, item->CTRLB);
}

bool DMAManager::add_entry(Word SADDR, Word DADDR, Word CTRLA, Word CTRLB) {
  if (size >= LLI_MAX_FRAMES) return false;
  volatile LLI *tail = get_next_blank_entry();
  tail->SADDR = SADDR;
  tail->DADDR = DADDR;
  tail->CTRLA = CTRLA;
  tail->CTRLB = CTRLB;
  size++;
  return true;
}

volatile LLI *DMAManager::get_next_blank_entry() {
  if (size >= LLI_MAX_FRAMES) return nullptr;
  return &dma_frames[size];
}

volatile LLI *DMAManager::get_entry(uint16_t idx) {
  if (idx >= size) return nullptr;
  return &dma_frames[idx];
}

void DMAManager::remove_entry(uint16_t idx) {
  if (idx >= size)
    return;

  if (idx < size - 1) { // Move every entry down
    uint16_t elements_moved = size - idx - 1;
    for (uint i = 0; i < elements_moved; i++) {
      dma_frames[idx + i] = dma_frames[idx + i + 1];
    }
  }

  size--;
}

volatile LLI *DMAManager::finalize() {
  for (int i = 0; i < size - 1; i++) {
    dma_frames[i].DSCR = (uint32_t)&dma_frames[i + 1];
  }
  dma_frames[size - 1].DSCR = 0;
  return dma_frames;
}

void DMAManager::clear_frames() {
  size = 0;
}

void DMAManager::reset() {
  clear_frames();
  transaction_data.reset();
}

uint32_t DMAManager::get_size() const {
  return size;
}

bool DMAManager::can_add_entries(uint32_t entries) const {
  return size + entries < LLI_MAX_FRAMES;
}

/*****************************
 * Frame Add Functions
 *****************************/

bool DMAManager::add_entry_cs_pin_toggle(bool state, uint8_t num_transfers) {
  return add_entry_pin_toggle(state, _csPin, &_csPinMask, num_transfers);
}

bool DMAManager::add_entry_pin_toggle(bool state, uint8_t pin, uint32_t *pin_mask_ptr, uint8_t num_transfers) {
  Pio *pin_descriptor = digitalPinToPort(pin);

  auto SADDR = (Word)pin_mask_ptr;
  Word DADDR;
  Word CTRLA = num_transfers | DMAC_CTRLA_SRC_WIDTH_WORD | DMAC_CTRLA_DST_WIDTH_WORD;
  Word CTRLB = DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_FIXED;

  if (state) {
    DADDR = (Word)&pin_descriptor->PIO_SODR;
  } else {
    DADDR = (Word)&pin_descriptor->PIO_CODR;
  }

  return add_entry(SADDR, DADDR, CTRLA, CTRLB);
}

bool DMAManager::add_entry_spi_transfer(volatile uint8_t *buf, size_t qty) {
  if (qty <= 0) return false;

  static uint8_t ff = 0XFF;
  uint32_t src_incr = DMAC_CTRLB_SRC_INCR_INCREMENTING;
  if (!buf) {
    buf = &ff;
    src_incr = DMAC_CTRLB_SRC_INCR_FIXED;
  }

  auto SADDR = (Word)buf;
  Word DADDR = (uint32_t)&SPI0->SPI_TDR;
  Word CTRLA = qty | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
  Word CTRLB = DMAC_CTRLB_FC_MEM2PER_DMA_FC |
               src_incr | DMAC_CTRLB_DST_INCR_FIXED;

  return add_entry(SADDR, DADDR, CTRLA, CTRLB);
}

bool DMAManager::add_entry_coord_bits(uint16_t coord, uint8_t coord_register) {
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
  return add_entry_spi_transfer(data_ptr, COORD_BUF_SPACE);
}

bool DMAManager::add_entry_spi_draw_pixels(volatile uint8_t *buf, size_t qty) {
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
  return ret1 && ret2;
}
