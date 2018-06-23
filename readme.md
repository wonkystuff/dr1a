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
written in Ruby. The generated
code (`calc.c`/`calc.h`) contains stuff like the wavetables and octave-lookup
information (stuff that is easily described mathematically
but you really don't want to type in). Basically, don't edit
the generated code directly, otherwise it will be overwritten
next time you run the generator!
* **Program *(`main.c`)*:** This is the main source file, and contains a couple of functions, the shape of which will be familiar to those with an Arduino background: `setup()` and `main()` (kind of like `loop()` but it never returns).
* **Interrupt Routine *(`isr.c`)*:** The actual sound is output from here, it is triggered by a timer running at 50kHz (the sample rate).

I hope that I've put sufficient comments in the code for a description not to be necessary here, but please ping me a message if there's stuff which needs clarifying

The code is built using 'make'. The ATTiny85 is programmed by issuing:
````
> make all
> make flashfuse
> make flashcode
````
`make all` builds the application; `make flashfuse` writes the fuse information to the ATTIny (clock selection etc.); `make flashcode` actually programs the chip.

## Hardware notes

ADC0 is used, and this shares a pin with `reset`. If the reset functionality is enabled then this ADC channel only has half of the range of the other channels. The default behaviour of this (C) code is that a high-voltage programmer is being used (I use a minipro TL866) and therefore the full range is available. The arduino code (on the master branch of this repo) does not make that assumption… In the case that the full range of control is needed, then R4 can be replaced by a wire link.

If you are not using a high-voltage programmer, I recommend switching to the master branch and using the Arduino build. You'll need to add a resistor in position R4 of the same value as the potentiometer…
