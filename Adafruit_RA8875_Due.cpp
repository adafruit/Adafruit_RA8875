#include "SpiDriver.h"

#if defined(ARDUINO_SAM_DUE) && USE_DMA_INTERRUPT

#include "Adafruit_RA8875.h"
#include "function_timings.h"

typedef volatile union _SADDR_Type {
  uint32_t *cs_mask;
  CoordEntryCommand *coord_entry;
  Word raw;
} SADDR_Type;

/**
 * Redef of the LLI as an entry in a row.
 * Uses SADDR_Type union for easier debugging of what's in the SADDR field
 */
typedef volatile struct _RowFrame {
  SADDR_Type SADDR;
  Word DADDR;
  CTRLA_Field CTRLA;
  CTRLB_Field CTRLB;
  Word DSCR;
} RowFrame;

typedef volatile struct _Row {
  RowFrame frames[FRAMES_PER_LINE];

  RowFrame *get_frame(uint8_t frame_idx) volatile {
    if (frame_idx < 0 || frame_idx >= FRAMES_PER_LINE) {
      return nullptr;
    }
    return (frames + frame_idx);
  }
} Row;

/**
 * Gets a chunk of LLIs and treats them like an entire row of pixels. Good for bulk operations
 * @param manager The DMA Manager
 * @param row Which row to get
 * @return The collection of frames as a row pointer, if within range. Nullptr otherwise
 */
static Row *get_row(DMAManager *manager, size_t row) {
  if (row * FRAMES_PER_LINE >= LLI_MAX_FRAMES) {
    return nullptr;
  }
  return (Row *)manager->get_entry(row * FRAMES_PER_LINE, true);
}

void Adafruit_RA8875::onDMAInterrupt() {
  RA_DEBUG_START(FLUSH);
  uint32_t interrupt = DMAC->DMAC_EBCISR;
  (void)interrupt;
  while ((SPI0->SPI_SR & SPI_SR_TXEMPTY) == 0) {}
  //leave RDR empty
  while (SPI0->SPI_SR & (SPI_SR_OVRES | SPI_SR_RDRF)) {
    SPI0->SPI_RDR;
  }
  RA_DEBUG_STOP(FLUSH);

  spiDriver.nextDMA();
}

/**
 * Delegate function to draw a batch of pixels from the larger buffer.
 * This operation takes 21 LLI Frames per line of pixels drawn, regardless of the length of the line.
 * CS Low                                                                   (1 frame)
 * setX (CurH0)                                (+ Dummy Transfers, CS High) (3 frames)
 * CS Low                                                                   (1 frame)
 * setX (CurH1)                                (+ Dummy Transfers, CS High) (3 frames)
 * CS Low                                                                   (1 frame)
 * setY (CurV0)                                (+ Dummy Transfers, CS High) (3 frames)
 * CS Low                                                                   (1 frame)
 * setY (curV1)                                (+ Dummy Transfers, CS High) (3 frames)
 * CS Low                                                                   (1 frame)
 * set command MWRC, datawrite, send pixels    (+ Dummy Transfers)          (3 frames)
 * CS High                                                                  (1 frame)
 * @param manager   The DMA Manager responsible for frames
 * @param data      Data with callbacks & function-specific data
 */
static void drawPixelsDMADelegateSlow(DMAManager *manager, DMA_Data *data) {
  RA_DEBUG_DELEGATE_INC();
  RA_DEBUG_START(DELEGATE);
  DrawAreaData *functionData = &data->functionData.drawAreaData;
  // Pointers for stuff that will be updated
  uint16_t *pixel_arr_start = functionData->colors;
  volatile uint32_t *pixels_left = &(functionData->num);
  volatile uint16_t *rows_completed = &(functionData->rows_completed);

  // Values that we need saved
  uint16_t area_width = functionData->width;

  // Reset frames
  manager->clear_frames();

  // Add frames for each line until full or out of pixels
  while (*pixels_left > 0 && manager->can_add_entries(FRAMES_PER_LINE)) {

    uint16_t y_row = functionData->y + (*rows_completed);

    // Write XY Coordinates

    manager->add_entry_cs_pin_toggle(LOW);
    manager->add_entry_coord_bits(functionData->x, RA8875_CURH0);
    // Keep CS Pin low until we're sure the data has transferred
    manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
    manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);

    manager->add_entry_cs_pin_toggle(LOW);
    manager->add_entry_coord_bits(functionData->x, RA8875_CURH1);
    manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
    manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);

    manager->add_entry_cs_pin_toggle(LOW);
    manager->add_entry_coord_bits(y_row, RA8875_CURV0);
    manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
    manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);

    manager->add_entry_cs_pin_toggle(LOW);
    manager->add_entry_coord_bits(y_row, RA8875_CURV1);
    manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
    manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);

    // Draw Pixels out

    uint16_t to_transfer = min(*pixels_left, area_width);
    auto start = (uint8_t *)(pixel_arr_start + ((*rows_completed) * area_width));

    manager->add_entry_cs_pin_toggle(LOW);
    manager->add_entry_spi_draw_pixels(start, sizeof(uint16_t) * to_transfer);
    manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
    manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);

    // Subtract pixels from total and increment the lines done
    *pixels_left -= to_transfer;
    *rows_completed += 1;
  }

  RA_DEBUG_STOP(DELEGATE);
}

/**
 * Delegate function to draw a batch of pixels from the larger buffer.
 * Uses quick row transforms instead of rewriting the entire buffer like the slow version.
 * This operation takes 21 LLI Frames per line of pixels drawn, regardless of the length of the line.
 * CS Low                                                                   (1 frame)
 * setX (CurH0)                                (+ Dummy Transfers, CS High) (3 frames)
 * CS Low                                                                   (1 frame)
 * setX (CurH1)                                (+ Dummy Transfers, CS High) (3 frames)
 * CS Low                                                                   (1 frame)
 * setY (CurV0)                                (+ Dummy Transfers, CS High) (3 frames)
 * CS Low                                                                   (1 frame)
 * setY (curV1)                                (+ Dummy Transfers, CS High) (3 frames)
 * CS Low                                                                   (1 frame)
 * set command MWRC, datawrite, send pixels    (+ Dummy Transfers)          (3 frames)
 * CS High                                                                  (1 frame)
 * @param manager   The DMA Manager responsible for frames
 * @param data      Data with callbacks & function-specific data
 */
static void drawPixelsDMADelegateRows(DMAManager *manager, DMA_Data *data) {
  RA_DEBUG_DELEGATE_INC();
  RA_DEBUG_START(DELEGATE);
  size_t cur_row = 0;
  DrawAreaData *functionData = &data->functionData.drawAreaData;
  // Pointers for stuff that will be updated
  uint16_t *pixel_arr_start = functionData->colors;
  volatile uint32_t *pixels_left = &(functionData->num);
  volatile uint16_t *rows_completed = &(functionData->rows_completed);

  // Values that we need saved
  uint16_t area_width = functionData->width;

  // Reset frames
  manager->clear_frames();

  // Add frames for each line until full or out of pixels
  while (*pixels_left > 0 && manager->can_add_entries(FRAMES_PER_LINE)) {

    uint16_t y_row = functionData->y + (*rows_completed);
    volatile Row *row = get_row(manager, cur_row++);

    // Write XY Coordinates

    row->get_frame(1)->SADDR.coord_entry->coord = functionData->x;
    row->get_frame(5)->SADDR.coord_entry->coord = (functionData->x >> 8);
    row->get_frame(9)->SADDR.coord_entry->coord = (y_row);
    row->get_frame(13)->SADDR.coord_entry->coord = (y_row >> 8);

    // Draw Pixels out

    uint16_t to_transfer = min(*pixels_left, area_width);
    auto start = (uint8_t *)(pixel_arr_start + ((*rows_completed) * area_width));

    volatile RowFrame *frame = row->get_frame(18);
    frame->SADDR.raw = (Word)start;
    frame->CTRLA.BTSIZE = (sizeof(uint16_t) * to_transfer);

    // Subtract pixels from total and increment the lines done
    *pixels_left -= to_transfer;
    *rows_completed += 1;
    manager->increment_size(FRAMES_PER_LINE);
  }

  RA_DEBUG_STOP(DELEGATE);
}

void
Adafruit_RA8875::drawPixelsAreaDMASlow(uint16_t *p, uint32_t num, int16_t x, int16_t y, int16_t width, void *cbData,
                                       void (*complete_cb)(void *)) {
  RA_DEBUG_START(FULL);
  RA_DEBUG_START(DRAW_FUNC);
  DMAManager *manager = getManager();
  DMA_Data *curData = manager->get_cur_data();
  DMAFunctionData *functionData = &curData->functionData;

  // Set operation and callback functions for testing & checking batches of frames
  curData->operation = DRAW_PIXELS_AREA;

  curData->is_complete = [](const DMAFunctionData &function_data) -> volatile bool {
    return function_data.drawAreaData.num <= 0;
  };

  curData->fetch_next_batch = [](DMAManager *manager, DMA_Data *data) -> volatile void {
    data->clear_working_data();
    drawPixelsDMADelegateSlow(manager, data);
  };

  curData->on_complete = [](SpiDriver *driver) -> volatile void {
    driver->deactivate();
  };

  // Fill callback data
  curData->callbackData.complete_cb = complete_cb;
  curData->callbackData.dataPtr = cbData;

  // Set basic data for this function to be used in the future
  functionData->drawAreaData.colors = p;
  functionData->drawAreaData.num = num;
  functionData->drawAreaData.x = applyRotationX(x);
  functionData->drawAreaData.y = applyRotationY(y);
  functionData->drawAreaData.width = width;
  functionData->drawAreaData.rows_completed = 0;

  // Set registers
  uint8_t dir = RA8875_MWCR0_LRTD;
  if (_rotation == 2) {
    dir = RA8875_MWCR0_RLTD;
  }
  writeReg(RA8875_MWCR0, (readReg(RA8875_MWCR0) & ~RA8875_MWCR0_DIRMASK) | dir);

  // Fetch batch, start
  RA_DEBUG_PAUSE(DRAW_FUNC);
  curData->fetch_next_batch(manager, curData);
  RA_DEBUG_START(DRAW_FUNC);
  spiDriver.activate();
  // Manager needs to be finalized before starting or the chain will not run correctly
  spiDriver.sendChain(manager->finalize());
  RA_DEBUG_STOP(DRAW_FUNC);
}

void Adafruit_RA8875::drawPixelsAreaDMA(uint16_t *p, uint32_t num, int16_t x, int16_t y, int16_t width, void *cbData,
                                        void (*complete_cb)(void *)) {
  RA_DEBUG_START(FULL);
  RA_DEBUG_START(DRAW_FUNC);
  DMAManager *manager = getManager();
  DMA_Data *curData = manager->get_cur_data();
  DMAFunctionData *functionData = &curData->functionData;

  // Set operation and callback functions for testing & checking batches of frames
  curData->operation = DRAW_PIXELS_AREA;

  curData->is_complete = [](const DMAFunctionData &function_data) -> volatile bool {
    return function_data.drawAreaData.num <= 0;
  };

  curData->fetch_next_batch = [](DMAManager *manager, DMA_Data *data) -> volatile void {
    drawPixelsDMADelegateRows(manager, data);
  };

  curData->on_complete = [](SpiDriver *driver) -> volatile void {
    driver->deactivate();
  };

  // Fill callback data
  curData->callbackData.complete_cb = complete_cb;
  curData->callbackData.dataPtr = cbData;

  // Set basic data for this function to be used in the future
  functionData->drawAreaData.colors = p;
  functionData->drawAreaData.num = num;
  functionData->drawAreaData.x = applyRotationX(x);
  functionData->drawAreaData.y = applyRotationY(y);
  functionData->drawAreaData.width = width;
  functionData->drawAreaData.rows_completed = 0;

  // Set registers
  uint8_t dir = RA8875_MWCR0_LRTD;
  if (_rotation == 2) {
    dir = RA8875_MWCR0_RLTD;
  }
  writeReg(RA8875_MWCR0, (readReg(RA8875_MWCR0) & ~RA8875_MWCR0_DIRMASK) | dir);

  // Write in blank data for the frame batcher to fill

  // Attempt to reuse frames if we want
  if (REUSE_DMA_FRAMES_IF_AVAILABLE && curData->last_operation == DRAW_PIXELS_AREA) {

    curData->storage_idx = curData->last_storage_idx;

  } else {                                              // Else create new frames
    manager->clear_frames();

    for (int i = 0; i < LINES_PER_DMA; i++) {

      // Write XY Coordinates
      manager->add_entry_cs_pin_toggle(LOW);
      manager->add_entry_coord_bits(0, RA8875_CURH0);
      // Keep CS Pin low until we're sure the data has transferred
      manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
      manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);

      manager->add_entry_cs_pin_toggle(LOW);
      manager->add_entry_coord_bits(0, RA8875_CURH1);
      manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
      manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);

      manager->add_entry_cs_pin_toggle(LOW);
      manager->add_entry_coord_bits(0, RA8875_CURV0);
      manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
      manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);

      manager->add_entry_cs_pin_toggle(LOW);
      manager->add_entry_coord_bits(0, RA8875_CURV1);
      manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
      manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);

      // Draw Pixels out

      manager->add_entry_cs_pin_toggle(LOW);
      manager->add_entry_spi_draw_pixels((uint8_t *)p, sizeof(uint16_t) * width);
      manager->add_entry_dummy_transfer(DMA_DUMMY_TRANSFERS);
      manager->add_entry_cs_pin_toggle(HIGH, DMA_CS_HIGH_TRANSFERS);
    }
  }

  // Fetch batch, start
  RA_DEBUG_PAUSE(DRAW_FUNC);
  curData->fetch_next_batch(manager, curData);
  RA_DEBUG_START(DRAW_FUNC);
  spiDriver.activate();
  // Manager needs to be finalized before starting or the chain will not run correctly
  spiDriver.sendChain(manager->finalize());
  RA_DEBUG_STOP(DRAW_FUNC);
}

#endif