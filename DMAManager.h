#ifndef ADAFRUIT_RA8875_DMAMANAGER_H
#define ADAFRUIT_RA8875_DMAMANAGER_H

#include "DMA_LLI.h"
#include <cstdint>
#include <Arduino.h>

/********************************/
/**
 * Guards to allow these values to be set at compile
 * */
/********************************/
#ifndef FRAMES_PER_LINE
#define FRAMES_PER_LINE 16
#endif
#ifndef LINES_PER_DMA
#define LINES_PER_DMA 8
#endif
#ifndef WORKING_DATA_PER_LINE
#define WORKING_DATA_PER_LINE 19
#endif

#define LLI_MAX_FRAMES (FRAMES_PER_LINE * LINES_PER_DMA)
#define WORKING_DATA_SIZE (WORKING_DATA_PER_LINE * LINES_PER_DMA)
#define COORD_BUF_SPACE 4
#define DMA_CS_HIGH_TRANSFERS 8

// Forward Declaration for class
class DMAManager;

class SpiDriver;

typedef volatile struct _CoordEntryCommand {
  uint8_t write_command;
  uint8_t write_reg;
  uint8_t data_write;
  uint8_t coord;
} CoordEntryCommand;

/**
 * Enum for operation defs.
 * The operation should always be changed from NONE to allow the last_value to be set correctly.
 * Some functions may rely on last_data to reuse LLI frames.
 */
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

typedef struct DMACallbackData {
  void *dataPtr;

  void (*complete_cb)(void *);
} DMACallbackData;

static void memset_volatile(volatile void *s, char c, size_t n) {
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
  DMAOperation last_operation;
  DMAFunctionData functionData;
  DMACallbackData callbackData;
  volatile uint16_t storage_idx;
  volatile uint16_t last_storage_idx;
  uint8_t volatile working_storage[WORKING_DATA_SIZE];

  bool volatile (*is_complete)(const DMAFunctionData &function_data);

  void volatile (*fetch_next_batch)(DMAManager *manager, DMA_Data *data);

  void volatile (*on_complete)(SpiDriver *spiDriver);


  inline void clear_working_data(bool full_clear = false) {
    last_storage_idx = full_clear ? 0 : storage_idx;
    storage_idx = 0;
  }

  inline void reset(bool full_reset = false) {
    clear_working_data(full_reset);
    memset_volatile(&functionData, '\0', sizeof(DMAFunctionData));
    last_operation = full_reset ? NONE : operation;
    operation = NONE;
    callbackData.dataPtr = nullptr;
    callbackData.complete_cb = nullptr;
    is_complete = nullptr;
    fetch_next_batch = nullptr;
    on_complete = nullptr;
  }

  inline bool can_add_working_data(size_t size) const {
    return ((storage_idx - 1) + size) < WORKING_DATA_SIZE;
  }

  inline volatile uint8_t *add_working_data(const uint8_t *buf, size_t size) {
    if (!can_add_working_data(size)) {
      return nullptr;
    }
    uint16_t cursorStart = storage_idx;
    for (size_t i = 0; i < size; i++) {
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
    reset(true);
  }

  /**
   * Copy the contents of the current LLI into the next in the list.
   * Currently **will not** preserve DSCR paths.
   */
  bool add_entry(volatile LLI *item);

  /**
   * Copy the contents of the current LLI into the next in the list.
   * Currently **will not** preserve DSCR paths.
   */
  bool add_entry(const volatile LLI &item);

  /**
   * Just increments the size field, useful if manually programming frames
   * */
  bool increment_size(size_t qty = 1) {
    if ((size + qty) > LLI_MAX_FRAMES) return false;
    size += qty;
    return true;
  }

  volatile LLI *get_next_blank_entry();

  volatile LLI *get_entry(size_t idx, bool force = false);

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

  void reset(bool full = false);

  size_t get_size() const;

  bool can_add_entries(uint32_t entries) const;

  DMA_Data *get_cur_data() {
    return &transaction_data;
  }

  /************************
   * General purpose methods for adding SPI Frames, PIO Frames, etc.
   ************************/
  WoReg *get_pio_reg(bool state, uint8_t pin);

  WoReg *get_cs_pio_reg(bool state) {
    return get_pio_reg(state, _csPin);
  }

  bool add_entry_pin_toggle(bool state, uint8_t pin, const uint32_t *pin_mask_ptr, size_t num_transfers = 2);

  bool add_entry_cs_pin_toggle(bool state, size_t num_transfers = 2);

  bool add_entry_spi_transfer(volatile uint8_t *buf, size_t qty);

  volatile uint8_t *add_entry_spi_draw_pixels(volatile uint8_t *buf, size_t qty);

  /**
   * Adds an entry to set the bits in an X/Y position register
   * @param coord The X or Y position
   * @param coord_register RA8875_CURH0, RA8875_CURH1, RA8875_CURV0, RA8875_CURV1
   * @return nullptr if there is no room to add the frame, or if the wrong register is provided. Returns a valid pointer otherwise.
   */
  volatile CoordEntryCommand *add_entry_coord_bits(uint16_t coord, uint8_t coord_register);

  /**
   * Accessible wrapper for adding data directly to the transaction buffer
   * @param buf The buffer
   * @param buf_size size
   * @return The starting point of the buffer in the working data struct
   */
  volatile uint8_t *add_transaction_data(const uint8_t *buf, size_t buf_size) {
    return transaction_data.add_working_data(buf, buf_size);
  }

private:
  DMA_Data transaction_data{};                /**< Callbacks & Data that needs to be stored for the entirety of the DMA operation*/
  volatile size_t size;                       /**< Cur Number of frames in buffer*/
  uint8_t _csPin;                             /**< ChipSelect Pin*/
  uint32_t _csPinMask;                        /**< Persistent mask for CS Pin, since it can't be obtained by DMA*/
  LLI volatile dma_frames[LLI_MAX_FRAMES];    /**< Constructed frames*/
};


#endif //ADAFRUIT_RA8875_DMAMANAGER_H
