# single

*single* is a development of *adc* where I’m concentrating on just generating a single oscillator, but
this will be a bit more feature-rich than the previous experiments:

* multiple selectable waveshapes;
* higher sample rate (at least double that of *adc* since we don’t need to toggle between two oscillators;
* … plus some other stuff :)

Source layout is the same as for the other projects; there is a code generator written in ruby to
generate the pitch-increment lookup table and the wavetables themselves, the time-critical code is done
as straight-line as possible.
