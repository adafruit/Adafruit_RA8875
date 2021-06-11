#ifndef ADAFRUIT_RA8875_DMAMANAGER_H
#define ADAFRUIT_RA8875_DMAMANAGER_H

#include <cstdint>
#include <Arduino.h>

#define FRAMES_PER_LINE 16
#define LINES_PER_DMA 20
#define LLI_MAX_FRAMES (FRAMES_PER_LINE * LINES_PER_DMA)
#define WORKING_DATA_PER_LINE 19
#define WORKING_DATA_SIZE (WORKING_DATA_PER_LINE * LINES_PER_DMA)
#define COORD_BUF_SPACE 4
#define DMA_CS_HIGH_TRANSFERS 8

#ifdef ARDUINO_SAM_DUE
typedef uint32_t Word;
#elif defined(__AVR__)
typedef uint16_t Word;
#endif

// Forward Declaration for class
class DMAManager;

class SpiDriver;

typedef volatile struct _LLI {
  _LLI();

  _LLI(volatile _LLI &);

  _LLI(_LLI const volatile &&);

  volatile _LLI &operator=(const volatile _LLI &lhs) volatile {
    this->SADDR = lhs.SADDR;
    this->DADDR = lhs.DADDR;
    this->CTRLA = lhs.CTRLA;
    this->CTRLB = lhs.CTRLB;
    this->DSCR = lhs.DSCR;
    return *this;
  }

  Word SADDR; /**< DMAC SADDR (Start Address)*/
  Word DADDR; /**< DMAC DADDR (Destination Address)*/
  Word CTRLA; /**< DMAC CTRLA*/
  Word CTRLB; /**< DMAC CTRLB*/
  Word DSCR;  /**< The next LLI Entry*/
} LLI;

typedef enum DMAOperation {
  NONE,
  DRAW_PIXELS_AREA
} DMAOperation;

/************************
 * Struct defs for data used in operations
 *
 ************************/

struct DrawAreaData {
  uint16_t *colors;
  volatile uint32_t num;
  int16_t x;
  int16_t y;
  int16_t width;
  volatile uint16_t rows_completed;
};

typedef union DMAFunctionData {
  DrawAreaData drawAreaData;
} DMAFunctionData;

void memset_volatile(volatile void *s, char c, size_t n) {
  char *p = (char *)s;
  while (n-- > 0) {
    *p++ = c;
  }
}

/************************
 * Data holder for a single operation.
 * Provides methods for setting up the next block, handling interrupts, returning on complete.
 ************************/

typedef struct DMA_Data {
  DMAOperation operation;
  DMAFunctionData functionData;
  volatile uint16_t storage_idx;
  volatile uint8_t working_storage[WORKING_DATA_SIZE];

  bool (*is_complete)(const DMAFunctionData &function_data);

  void (*fetch_next_batch)(DMAManager *manager, DMA_Data *data);

  void (*on_complete)(SpiDriver *spiDriver);

  void (*complete_cb)();

  inline void clear_working_data() {
    storage_idx = 0;
  }

  inline void reset() {
    clear_working_data();
    memset_volatile(&functionData, '\0', sizeof(DMAFunctionData));
  }

  inline bool can_add_working_data(size_t size) const {
    return ((storage_idx - 1) + size) < WORKING_DATA_SIZE;
  }

  inline volatile uint8_t *add_working_data(const uint8_t *buf, size_t size) {
    if (!can_add_working_data(size)) {
      Serial.println("HELP");
      return nullptr;
    }
    uint16_t cursorStart = storage_idx;
    for (int i = 0; i < size; i++) {
      working_storage[storage_idx++] = buf[i];
    }

    return &working_storage[cursorStart];
  }
} DMA_Data;


class DMAManager {
public:
  explicit DMAManager(uint8_t csPin) : size(0), dma_frames() {
    _csPin = csPin;
    _csPinMask = digitalPinToBitMask(csPin);
    reset();
  }

  /**
   * Copy the contents of the current LLI into the next in the list.
   * Currently **will not** preserve DSCR paths.
   */
  bool add_entry(LLI *item);

  bool add_entry(Word SADDR, Word DADDR, Word CTRLA, Word CTRLB);

  volatile LLI *get_next_blank_entry();

  volatile LLI *get_entry(uint16_t idx);

  inline volatile LLI *get_last() {
    return get_entry(get_size() - 1);
  }

  void remove_entry(uint16_t idx);

  /**
   * Writes DSCR addresses into the chain.
   * The structure isn't quite a linkedlist since it's all contiguous, so we can save time by not writing the `next` pointers
   * until we're done with the entire chain. This way if an element needs to be removed, all the `next` pointers won't need to be
   * rewritten.
   * This must be called before passing the beginning entry to DMA
   */
  volatile LLI *finalize();

  void clear_frames();

  void reset();

  uint32_t get_size() const;

  bool can_add_entries(uint32_t entries) const;

  DMA_Data *get_cur_data() {
    return &transaction_data;
  }

  void next_operation();

  /************************
   * General purpose methods for adding SPI Frames, PIO Frames, etc.
   ************************/
  bool add_entry_pin_toggle(bool state, uint8_t pin, uint32_t *pin_mask_ptr, uint8_t num_transfers = 2);

  bool add_entry_cs_pin_toggle(bool state, uint8_t num_transfers = 2);

  bool add_entry_spi_transfer(volatile uint8_t *buf, size_t qty);

  bool add_entry_spi_draw_pixels(volatile uint8_t *buf, size_t qty);

  /**
   * Adds an entry to set the bits in an X/Y position register
   * @param coord The X or Y position
   * @param coord_register RA8875_CURH0, RA8875_CURH1, RA8875_CURV0, RA8875_CURV1
   * @return false if there is no room to add the frame, or if the wrong register is provided. True otherwise
   */
  bool add_entry_coord_bits(uint16_t coord, uint8_t coord_register);

private:
  DMA_Data transaction_data{};                /**< Callbacks & Data that needs to be stored for the entirety of the DMA operation*/
  volatile uint32_t size;                     /**< Cur Number of frames in buffer*/
  uint8_t _csPin;                             /**< ChipSelect Pin*/
  uint32_t _csPinMask;                        /**< Persistent mask for CS Pin, since it can't be obtained by DMA*/
  LLI volatile dma_frames[LLI_MAX_FRAMES]{};  /**< Constructed frames*/
};


#endif //ADAFRUIT_RA8875_DMAMANAGER_H
