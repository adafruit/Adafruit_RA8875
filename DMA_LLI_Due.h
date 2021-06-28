#ifndef ADAFRUIT_RA8875_DMA_LLI_DUE_H
#define ADAFRUIT_RA8875_DMA_LLI_DUE_H

#include <Arduino.h>

typedef uint32_t Word;

/**
 * Bitfield for CTRLA register.
 */
typedef volatile union _CTRLA_Field {
  _CTRLA_Field() : raw(0) {};

  _CTRLA_Field(Word raw) : raw(raw) {};

  struct {
    uint BTSIZE: 16;
    uint SCSIZE: 3;
    uint BLANK1: 1;
    uint DCSIZE: 3;
    uint BLANK2: 1;
    uint SRC_WIDTH: 2;
    uint BLANK3: 2;
    uint DST_WIDTH: 2;
    uint BLANK4: 1;
    uint DONE: 1;
  };
  Word raw;
} CTRLA_Field;

/**
 * Bitfield for CTRLB Register
 */
typedef volatile union _CTRLB_Field {
  _CTRLB_Field() : _CTRLB_Field(0) {};

  explicit _CTRLB_Field(Word raw) : raw(raw) {};

  struct {
    uint BLANK1: 16;
    uint SRC_DSCR: 1;
    uint BLANK2: 3;
    uint DST_DSCR: 1;
    uint FC: 3;
    uint SRC_INCR: 2;
    uint BLANK3: 2;
    uint DST_INCR: 2;
    uint IEN: 1;
    uint BLANK4: 1;
  };
  Word raw;
} CTRLB_Field;

typedef volatile struct _LLI {
  _LLI();

  _LLI(volatile _LLI &);

  _LLI(_LLI const volatile &&);

  _LLI(Word SADDR, Word DADDR, Word CTRLA, Word CTRLB);

  Word SADDR;           /**< DMAC SADDR (Start Address)*/
  Word DADDR;           /**< DMAC DADDR (Destination Address)*/
  CTRLA_Field CTRLA;    /**< DMAC CTRLA*/
  CTRLB_Field CTRLB;    /**< DMAC CTRLB*/
  Word DSCR;            /**< The next LLI Entry*/
} LLI;

#endif