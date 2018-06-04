# dr1.a

*dr1.a* is the first _real_ audio generator project, designed to be used by Orlando Ferguson for the first gig in their third phase. Since this is a further development of *single*, it shares some features:

* multiple selectable waveshapes (via ADC1);
* 50kHz sample rate (250kHz PWM);

In addition:

* _Perturbation Control_: introduces a random switching of wavetable (amount is varied by a control connected to ADC0);
* _Hard Sync_: The oscillator is periodically hard-reset to phase 0, with independent control of both frequencies (this can generate some quite brutal discontinuities, so lots of high harmonics generated. Can sound a bit like a filter sweep (frequency controls connected to ADC2 and ADC3);

## Source layout

The project consists of a set of files, one of which is generated:

* **Generator *(calc.rb)*:** The code generator is
written in Ruby. The generated
code contains stuff like the wavetables and octave-lookup
information (stuff that is easily described mathematically
but you really don't want to type in). Basically, don't edit
the generated code directly, otherwise it will be overwritten
next time you run the generator!

* **Program *(dr1a.ino)*:** This is an arduino-file, and
contains three basic blocks: setup, loop and interrupt
routine. The sound output is done in the interrupt. I hope
that I've put sufficient comments in the code for a
description not to be necessary here…

## Hardware notes

ADC1 is used, and this shares a pin with `reset`. If the
reset functionality is enabled then this ADC channel only
has half of the range of the other channels.

If you are not using a 'high voltage' programmer, then this
is likely to be the case and R4 should be used. In the case
that a high-voltage programmer is being used, the reset pin
can be disabled and R4 replaced by a wire link. 
