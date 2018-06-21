/*
 * main.c (dr1.a remake)
 *
 * Created: 04/06/2018
 * Author: johna
 */

#include "calc.h"
#define NUM_ADCS (4)

// Base-timer is running at 8MHz
#define F_TIM (8000000L)

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

void
setup()
{
  PLLCSR |= _BV(PLLE);                // Enable 64 MHz PLL
  for(int i=0;i<10000;i++)
      ;             // Stabilize
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
  DDRB  = _BV(PB0) | _BV(PB1);  // signalling and PWM output pin respectively

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

  DIDR0 = _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D) | _BV(ADC3D);  // disable digital pin attached to ADC channels

  ADMUX  = 0;                       // select the mux for ADC0
  ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADEN);           //ADC Prescalar set to 64 - 125kHz@8MHz Enable ADC
  while (ADCSRA & _BV(ADSC) )
    { } // wait till conversion complete

  pi = 1;
  pi_sync = 1;
  // enable all the interrupts otherwise nothing happens...
  sei();
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
  static uint8_t  perturb = 0;
  static uint8_t  ws=0;

  // wait for the conversion to complete:
  while (ADCSRA & _BV(ADSC) )
    ;
  uint16_t  adcVal = (ADCL|(ADCH << 8));  // read 16 bits from the ADC

  switch(adcNum)
  {
    case 0:  // ADC 0 is on physical pin 1
      // The reset pin is inactive here, so we can use the full range

      // Perturb the main waveform randomly, but with a degree
      // of control
      adcVal = adcVal >> 2; // move into 8 bits
      if (adcVal > 16)      // give us a bit of a dead zone
      {
        if (--perturb == 0)
        {
          perturb = rnd();
          if (perturb < adcVal)
          {
            ws = perturb;
          }
        }
      }
      break;
    case 1:   // ADC 1 is on physical pin 7
      waveSelect = (ws + (adcVal >> 7)) & 0x07;          // gives us 0-7
      wave1 = waves[waveSelect >> 1];                       // 0-3
      wave2 = waves[(waveSelect >> 1) + (waveSelect & 1)];  // 0-4
      break;
    case 2:   // ADC 2 is on physical pin 3
      pi_sync = pgm_read_word(&octaveLookup[adcVal]);
      break;
    case 3:   // ADC 3 is on physical pin 2
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

int main(void)
{
  setup();
  while(1)
  {
    loop();
  }
}
