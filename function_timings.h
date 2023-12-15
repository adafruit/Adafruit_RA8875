#ifndef ADAFRUIT_RA8875_FUNCTION_TIMINGS_H
#define ADAFRUIT_RA8875_FUNCTION_TIMINGS_H

#include <cstdint>
#include "SpiConfig.h"

#define RA_DEBUG_PRINT_TIMINGS() (print_debug_timings_due())

#if PRINT_DMA_DEBUG

#define PRINTOUT_LIMIT 20

#define RA_CLEAR_TIMINGS() \
  if(!ra_enable_debug) {                                             \
    for(int i = 0; i < TimingDebugType::TIMING_ITEM_COUNT; i++) { \
      timing_starts[i] = 0;                                       \
      avg_micros_taken[i] = 0;                                    \
      avg_counters[i] = 0;                                        \
    }                                                             \
    total_interrupts = 0;                                         \
    total_delegates = 0;                                          \
  }

#define RA_SET_DEBUG(state)                                       \
  RA_CLEAR_TIMINGS();                                             \
  ra_enable_debug = state

#define RA_GET_DEBUG_STATE() (ra_enable_debug)

#define RA_DEBUG_START(type) \
  timing_starts[TimingDebugType::type] = micros()

#define RA_DEBUG_PAUSE(type) \
  avg_micros_taken[TimingDebugType::type] += (micros() - timing_starts[TimingDebugType::type])

#define RA_DEBUG_STOP(type) \
  RA_DEBUG_PAUSE(type);     \
  avg_counters[TimingDebugType::type]++

#define RA_DEBUG_AVERAGE(type) \
  avg_counters[TimingDebugType::type] == 0 ? 0 : avg_micros_taken[TimingDebugType::type] / avg_counters[TimingDebugType::type]

#define RA_DEBUG_INTERRUPT_INC() (total_interrupts++)

#define RA_DEBUG_DELEGATE_INC() (total_delegates++)

typedef enum TimingDebugType {
  FULL = 0,
  DELEGATE,
  DRAW_FUNC,
  SPI_TIMING,
  FLUSH,
  INTERRUPT,
  TIMING_ITEM_COUNT

} TimingDebugType;

extern volatile uint32_t total_interrupts;
extern volatile uint32_t total_delegates;
extern volatile uint32_t timing_starts[TimingDebugType::TIMING_ITEM_COUNT];
extern volatile double avg_micros_taken[TimingDebugType::TIMING_ITEM_COUNT];
extern volatile int avg_counters[TimingDebugType::TIMING_ITEM_COUNT];
extern char char_buf[256];
extern bool ra_enable_debug;

void print_debug_timings_due();

#else

#define RA_CLEAR_TIMINGS()
#define RA_SET_DEBUG(state)
#define RA_GET_DEBUG_STATE() (false)
#define RA_DEBUG_START(type)
#define RA_DEBUG_PAUSE(type)
#define RA_DEBUG_STOP(type)

#define RA_DEBUG_INTERRUPT_INC()
#define RA_DEBUG_DELEGATE_INC()

inline void print_debug_timings_due() {

}

#endif // PRINT_DMA_DEBUG

#endif //ADAFRUIT_RA8875_FUNCTION_TIMINGS_H
