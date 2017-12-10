 #include "calc.h"
#define NUM_ADCS (4)

// Base-timer is running at 8MHz
#define F_TIM (8000000L)

// Fixed value to start the ADC
// enable ADC, start conversion, prescaler = /64 gives us an ADC clock of 8MHz/64 (125kHz)
#define ADCSRAVAL ( _BV(ADEN) | _BV(ADSC) | _BV(ADPS2) | _BV(ADPS1)  | _BV(ADIE) )

// Remember(!) the input clock is 64MHz, therefore all rates
// are relative to that.
// let the preprocessor calculate the various register values 'coz
// they don't change after compile time
#if ((F_TIM/(SRATE)) < 255)
#define T1_MATCH ((F_TIM/(SRATE))-1)
#define T1_PRESCALE _BV(CS00)  //prescaler clk/1 (i.e. 8MHz)
#else
#define T1_MATCH (((F_TIM/8L)/(SRATE))-1)
#define T1_PRESCALE _BV(CS01)  //prescaler clk/8 (i.e. 1MHz)
#endif


#define OSCOUTREG (OCR1A)

const uint8_t     *waves[5];  // choice of wavetable
const uint8_t     *wave1;     // which wavetable will this oscillator use?
const uint8_t     *wave2;     // which wavetable will this oscillator use?
uint16_t          phase;      // The accumulated phase (distance through the wavetable)
uint16_t          pi;         // wavetable current phase increment (how much phase will increase per sample)
uint16_t          phase_sync; // The accumulated phase of the (virtual) sync oscillator (distance through the wavetable)
uint16_t          pi_sync;    // sync oscillator current phase increment (how much phase will increase per sample)

void setup()
{
  PLLCSR |= _BV(PLLE);                // Enable 64 MHz PLL
  delayMicroseconds(100);             // Stabilize
  while (!(PLLCSR & _BV(PLOCK)));     // Wait for it...
  PLLCSR |= _BV(PCKE);                // Timer1 source = PLL

  ///////////////////////////////////////////////
  // Set up Timer/Counter1 for 250kHz PWM output
  TCCR1 = 0;                  // stop the timer
  TCNT1 = 0;                  // zero the timer
  GTCCR = _BV(PSR1);          // reset the prescaler
  TCCR1 = _BV(PWM1A) | _BV(COM1A1) | _BV(COM1A0) | _BV(CS10);
  OCR1C = 255;
  OCR1A = 128;                // start with 50% duty cycle on the PWM
  pinMode(PB1, OUTPUT);       // PWM output pin


  waves[0] = sine;
  waves[1] = triangle;
  waves[2] = sq;
  waves[3] = ramp;
  waves[4] = sine;
  ///////////////////////////////////////////////
  // Set up Timer/Counter0 for sample-rate ISR
  TCCR0B = 0;                 // stop the timer (no clock source)
  TCNT0 = 0;                  // zero the timer

  TCCR0A = _BV(WGM01);        // CTC Mode
  TCCR0B = T1_PRESCALE;
  OCR0A = T1_MATCH;           // calculated match value
  TIMSK |= _BV(OCIE0A);

  ///////////////////////////////////////////////
  // Set up the ADC
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  //DIDR0 = _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D) | _BV(ADC3D);  // disable digital pin attached to ADC channels
  ADMUX  = 0;                       // select the mux for ADC0
  //ADCSRA = ADCSRAVAL;             // enable the ADC, set prescaler and start a conversion

  pinMode(PB0, OUTPUT);       // signalling pin

  pi = 1;
  pi_sync = 1;
}


// See http://doitwireless.com/2014/06/26/8-bit-pseudo-random-number-generator/
uint8_t rnd()
{
  static uint8_t r = 0x23;
  uint8_t lsb = r & 1;
  r >>= 1;
  r ^= (-lsb) & 0xB8;
  return r;
}

#define DATAWHEEL (1)

// There are no real time constraints here, this is an idle loop after
// all...
void loop()
{
  static uint8_t  adcNum=0;                // declared as static to limit variable scope
  static uint8_t  waveSelect;
  uint16_t        adcVal = analogRead(adcNum);  // Get the next adc value
  static uint8_t  perturb = 0;
  static uint8_t  ws=0;

  switch(adcNum)
  {
    case 0: // reduced range ~ 512-1023
      
      // Perturb the main waveform randomly, but with a degree
      // of control
      if (adcVal > 768) {
        if (--perturb == 0)
        {
          perturb = rnd();
          if (perturb < (adcVal-768)) // shift adcVal into 8 bit range
          {
            ws = perturb;
            //phase = 0;
          }
        }
      }
      break;
    case 1:
      waveSelect = (ws + (adcVal >> 7)) & 0x07;          // gives us 0-7
      wave1 = waves[waveSelect >> 1];                       // 0-3
      wave2 = waves[(waveSelect >> 1) + (waveSelect & 1)];  // 0-4
      break;
    case 2:
      pi_sync = pgm_read_word(&octaveLookup[adcVal]);
      break;
    case 3:
      pi = pgm_read_word(&octaveLookup[adcVal]);
      break;
  }  

  // next time we're dealing with a different channel; calculate which one:
  adcNum++;
  adcNum %= NUM_ADCS;

  // Start the ADC off again, this time for the next oscillator
  // it turns out that simply setting the MUX to the ADC number (0-3)
  // will select that channel, as long as you don't want to set any other bits
  // in ADMUX of course. Still, that's simple to do if you need to.
  ADMUX  = adcNum;      // select the correct channel for the next conversion
  ADCSRA |= _BV(ADSC);  // ADCSRAVAL;
}

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

