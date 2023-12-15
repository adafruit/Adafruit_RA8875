#include "function_timings.h"

#if PRINT_DMA_DEBUG

#include <Arduino.h>
#include "DMAManager.h"

// TODO add defines to prevent init for these things
volatile uint32_t printout_count = 0;
volatile uint32_t total_interrupts = 0;
volatile uint32_t total_delegates = 0;
volatile uint32_t timing_starts[TimingDebugType::TIMING_ITEM_COUNT]{};
volatile double avg_micros_taken[TimingDebugType::TIMING_ITEM_COUNT]{};
volatile int avg_counters[TimingDebugType::TIMING_ITEM_COUNT]{};
char char_buf[256];
bool ra_enable_debug = false;

// The size of this debug actually takes up more than the size of the default due Serial Buffer.
// Needs SERIAL_BUFFER_SIZE in RingBuffer.h set to >=256 to use
void print_debug_timings_due() {
#if (SERIAL_BUFFER_SIZE >= 256)
  printout_count++;
  if (RA_GET_DEBUG_STATE() && printout_count >= PRINTOUT_LIMIT) {
    printout_count = 0;
    RA_DEBUG_STOP(FULL);
    double del_time = RA_DEBUG_AVERAGE(DELEGATE);
    double spi_time = RA_DEBUG_AVERAGE(SPI_TIMING);
    double flush_time = RA_DEBUG_AVERAGE(FLUSH);
    double draw_func_time = RA_DEBUG_AVERAGE(DRAW_FUNC);
    double interrupt_total = RA_DEBUG_AVERAGE(INTERRUPT);
    uint32_t grand_total = RA_DEBUG_AVERAGE(FULL);
    snprintf(char_buf, 256,
             "\r\nDraw Function avg (no delegate): %.1f us\r\nSPI avg: %.2f us (%.2f us per line)\r\nFlush avg: %.1f us\r\nDelegate avg: %.2f us\r\nAvg Interrupt Time: %.1f us\r\nTotal Draw time: %lu us",
             draw_func_time, spi_time, spi_time / LINES_PER_DMA, flush_time, del_time, interrupt_total, grand_total);
    Serial.println(char_buf);
    snprintf(char_buf, 256, "Delegate + Interrupt + Method Time: %.1f us",
             (del_time + interrupt_total + draw_func_time));
    Serial.println(char_buf);

    RA_CLEAR_TIMINGS();
  }
#endif
}

#endif // PRINT_DMA_DEBUG