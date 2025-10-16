#include <avr/io.h> // Includes Hardware registers like DDRD and TCCR
#include <avr/interrupt.h> // Includes the interups


// Global variables

volatile uint16_t period; // Volatile so it can change anytime


void setup() {
  // put your setup code here, to run once:
  DDRD = 0; // set port D to input
  TCCR1A = 0 //Set 16 bit timer to just count up
  TCCR1B = 1 // no prescaling
  TCNT1 = 0 //Start counting frrom 0 

  

  sei();


}

void loop() {
  // put your main code here, to run repeatedly:

}
