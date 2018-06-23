# dr1.a

*dr1.a* is the first _real_ audio generator project, designed to be used by [Orlando Ferguson for the first gig in their third phase](https://youtu.be/1df9g67_uQI).

Headline Features:
* 8-bit PWM output (at 250kHz, via OC1A);
* 50kHz sample rate.
* multiple selectable waveshapes (via ADC1);
* _Perturbation Control_: introduces a random switching of wavetable (amount is varied by a control connected to ADC0);
* _Hard Sync_: The fundamental oscillator is periodically hard-reset to phase 0, with independent control of both fundamental and sync frequencies (ADC3 and ADC2 respectively). This can generate some quite brutal discontinuities, so lots of high harmonics generated.

## Source layout

The project consists of a set of files, two of which are generated:

* **Generator *(`calc.rb`)*:** The code generator is
written in Ruby. The generated code (`calc.ino`/`calc.h`) contains stuff like the wavetables and octave-lookup information (stuff that is easily described mathematically but you really don't want to type in). Basically, don't edit the generated code directly, otherwise it will be overwritten next time you run the generator!

* **Program *(`dr1a.ino`)*:** This is an arduino-file, and contains three basic blocks: setup, loop and interrupt routine. The sound output is done in the interrupt.

I hope that I've put sufficient comments in the code for a description not to be necessary here, but please ping me a message if there's stuff which needs clarifying.

## Hardware notes

ADC0 is used, and this shares a pin with `reset`. If the reset functionality is enabled then this ADC channel only has half of the range of the other channels. The default behaviour of this (Arduino) code is that a low-voltage programmer is being used and therefore only half of the range is available. The C code (on the `MakeVersion` branch of this repo) does not make that assumption… In the case that the full range of control is needed, R4 must be replaced by a wire link.

When using the half-range, R4 should be equal to the value of the potentiometer feeding ADC0 (e.g. 10k as per schematics).
