// Low power attiny example
// which reads the battery voltage level
//
// tvijlbrief@gmail.com 2012

#define BODS 7                   //BOD Sleep bit in MCUCR
#define BODSE 2                  //BOD Sleep enable bit in MCUCR

#include <avr/sleep.h>
#include <avr/wdt.h>

//#ifndef WDTCR // Missing for attiny44, 84
//#define WDTCR   _SFR_IO8(0x21)
//#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// set system into the sleep state 
// system wakes up when watchdog is timed out
void system_sleep() {
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
  sbi(ACSR,ACD);                    //disable the analog comparator

  //uint8_t mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
  //uint8_t mcucr2 = mcucr1 & ~_BV(BODSE);
  //MCUCR = mcucr1;
  //MCUCR = mcucr2;

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out 

  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
  cbi(ACSR,ACD); 
}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {
#if 1
  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  _WD_CONTROL_REG |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  _WD_CONTROL_REG = bb;
  _WD_CONTROL_REG |= _BV(WDIE);
#else
  wdt_enable(ii);
#endif
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  //wdt_disable();
  _WD_CONTROL_REG|=B01000000; // prevent reset if wakeup is too long
}


// Led pin:
int led = 9;

const int vPin= 1; // Measures battery level


// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void sleepDelay(unsigned n, boolean off= true)
{
  if (n < 16)
    delay(n);
  else {
    byte v= 0;
    while (n>>=1)
      v++;
    v-= 4;
    if (off) {
      pinMode(led,INPUT); // set all used port to input to save power
    }
    byte l= 1<<(v-9);
    do {
      setup_watchdog(v);
      system_sleep();
    } 
    while (--l != 0);
    pinMode(led,OUTPUT);
  }
}

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  analogReference(INTERNAL);  // Set the aref to the internal 1.1V reference
}


void blinkN(uint8_t n, int l= led)
{
  for (uint8_t i= 0; i < n; i++) {
    digitalWrite(l, HIGH);
    sleepDelay(32,false);
    digitalWrite(l, LOW);
    sleepDelay(512);
  }
}

void blinkFastN(uint8_t n, int l= led)
{
  for (uint8_t i= 0; i < n; i++) {
    digitalWrite(l, HIGH);
    sleepDelay(32,false);
    digitalWrite(l, LOW);
    sleepDelay(64);
  }
}

// the loop routine runs over and over again forever:
void loop() {
  unsigned v = analogRead(vPin);
  v= v * 343L / 775L; // 0.01 volts
  blinkN((v-200)/10);

  sleepDelay(8192);
}



