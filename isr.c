#include "calc.h"

#define OSCOUTREG (OCR1A)
extern const uint8_t     *wave1;     // which wavetable will this oscillator use?
extern const uint8_t     *wave2;     // which wavetable will this oscillator use?
extern uint16_t          phase;      // The accumulated phase (distance through the wavetable)
extern uint16_t          phase_sync; // The accumulated phase of the (virtual) sync oscillator (distance through the wavetable)
extern uint16_t          pi;         // wavetable current phase increment (how much phase will increase per sample)
extern uint16_t          pi_sync;    // sync oscillator current phase increment (how much phase will increase per sample)

// deal with oscillator
ISR(TIM0_COMPA_vect)
{
  // useful debug indicator to see if the sample rate is correct
  // PORTB ^= 1;

  // increment the phase counter
  phase += pi;

  uint16_t old_sync = phase_sync;
  phase_sync += pi_sync;
  if (phase_sync < old_sync)
  {
    phase = 0;
  }

  // By shifting the 16 bit number by 6, we are left with a number
  // in the range 0-1023 (0-0x3ff)
  uint16_t p = (phase) >> FRACBITS;

  // look up the output-value based on the current phase counter (truncated)

  // to save wavetable space, we play the wavetable forward (first half),
  // then backwards (and inverted)
  uint16_t ix = p < WTSIZE ? p : ((2*WTSIZE-1) - p);

  uint8_t s1 = pgm_read_byte(&wave1[ix]);
  uint8_t s2 = pgm_read_byte(&wave2[ix]);
  uint8_t s = s1 + s2;

  // invert the wave for the second half
  OSCOUTREG = p < WTSIZE ? -s : s;
}
