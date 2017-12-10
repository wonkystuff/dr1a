# dr1.a

*dr1.a* is the first _real_ audio generator project, designed to be used by Orlando Ferguson for the first gig in their third phase. Since this is a further development of *single*, it shares some features:

* multiple selectable waveshapes (via ADC1);
* 50kHz sample rate (250kHz PWM);

In addition:

* _Perturbation Control_: introduces a random switching of wavetable (amount is varied by a control connected to ADC0);
* _Hard Sync_: The oscillator is periodically hard-reset to phase 0, with independent control of both frequencies (this can generate some quite brutal discontinuities, so lots of high harmonics generated. Can sound a bit like a filter sweep (frequency controls connected to ADC2 and ADC3);

Source layout is the same as for the other projects; there is a code generator written in ruby to generate the pitch-increment lookup table and the wavetables themselves, the time-critical code is done as straight-line as possible.
