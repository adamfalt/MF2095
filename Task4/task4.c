#define F_CPU 16000000UL   // CPU Clock speed
#include <avr/io.h>        // For register names
#include <avr/interrupt.h> // For interupts

#include <stdint.h>        // ---- NEW for Task 4 ----

// (kept) UART helpers
static void uart_init(uint32_t baud)
{                                                // UART at 9600 baud ((Universal Asynchronous Receiver/Transmitter) for serial comunicarion)
    uint16_t ubrr = (F_CPU / (16UL * baud)) - 1; // Compute the UART Baud rate setting ubrr
    UBRR0H = (uint8_t)(ubrr >> 8);               // Load the baud rate setting into the baud rate registers (High byte(top 8 bits))
    UBRR0L = (uint8_t)(ubrr & 0xFF);             // Load the baud rate setting into the baud rate registers (Low byte(bottom 8 bits))
    UCSR0A = 0;                                  // Clears UART Control and Status Register A.
    UCSR0B = (1 << TXEN0);                       // Turn on transmitter  (TX)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);      // Fram format to 8 data bits, no parity, 1 stop(8N1)
}

static void uart_putc(char c)
{
    while (!(UCSR0A & (1 << UDRE0)))
    {
    } // Wait until the UART’s data register is empty, then write one character.
    UDR0 = c;
}

static void uart_puts(const char *s)
{
    while (*s)
    {
        uart_putc(*s++); // Send a string, charecter by character
    }
}

static void uart_putu32(uint32_t v)
{
    char buf[10];
    int i = 0;
    if (v == 0)
    { // If the number is zero, just print it and stop
        uart_putc('0');
        return;
    }
    // Build it backwards
    while (v && i < (int)sizeof(buf))
    {                              // v!=0 and there is space in the buffer
        buf[i++] = '0' + (v % 10); // Gives the last number of v and makes in to a char.
        v /= 10;                   // Removes the last number from v (interger)
    }

    while (i--)
    {
        uart_putc(buf[i]); // send it in correct order
    }
}

// Measurement state (microseconds)
// Volatile bc ISRs (Interrupt Service Routine) write these and main reads them
volatile unsigned long T_up = 0;      // time of last rising edge (us)
volatile unsigned long T_down = 0;    // time of last falling edge (us)
volatile unsigned long T_period = 0;  // period (us)
volatile unsigned long T_high = 0;    // high time (us)
volatile unsigned long overflows = 0; // counts 4 ms “ticks” in Timer1 CTC
volatile uint8_t Pin_prev = 0;        // previous input level (0/1)


// Button/LED/ADC state
volatile uint8_t mode_flag = 0;     // toggled by button (INT0) on D2
volatile uint16_t adc_raw = 0;      // last ADC sample (0..1023)
#define ADC_THRESHOLD 600           // simple threshold for LED action

ISR(TIMER1_COMPA_vect)
{
    overflows++; // 4 ms elapsed
}

ISR(PCINT0_vect)
{ // runs whenever D9 changes (low -> high or high -> low).

    unsigned long T_current = (overflows * 4000UL) + (TCNT1 / 16UL); // Timestamp in us
    uint8_t Pin_current = (PINB >> PB1) & 0x01u;                     // read D9 level (1 high, 0 low)

    if (Pin_prev < Pin_current)
    { // Detect rising edge

        if (T_up != 0)
        {
            T_period = T_current - T_up; // Calculate period  (Not for the first rise)
        }
        T_up = T_current; // remembers this rise
    }
    else if (Pin_prev > Pin_current)
    { // Detect falling edge

        T_down = T_current;
        if (T_down >= T_up)
        {
            T_high = T_down - T_up; // calculate high time(time from last rise to fall)
        }
    }
    Pin_prev = Pin_current; // Store pin level for next edge
}


ISR(INT0_vect)
{
    mode_flag ^= 1; //Toggles the flag whith button press
}


// ADC conversion complete interrupt store reading, update LED and PWM
ISR(ADC_vect){
    adc_raw = ADC;                      // 10-bit result
    if (adc_raw >= ADC_THRESHOLD)
    {
        // Turn LED ON when above threshold
        PORTB |= (1 << PB5);            // D13 high
    }
    else
    {
        PORTB &= ~(1 << PB5);           // D13 low
    }

    OCR0A = (uint8_t)(adc_raw >> 2); //Maps 10 bit value to 8 bit by bitshifting right

    ADCSRA |= (1 << ADSC); 
}

int main(void)
{

    DDRD |= (1 << DDD6);                                  // make D6 as output
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00); // in inverting fast PWM
    TCCR0B = (1 << CS01);                                 // prescaler /8 (f = F_CPU / (presc × 256)) -> PWM freq = ca 7.8kHz
    // TCCR0B = (1<<CS01) | (1<<CS00);                       // Uncomment for /64 -> ca 976.6 Hz
    OCR0A = 127; // ~50% duty ((127+1)/256)

    DDRB &= ~(1 << DDB1);    // make D9 as input
    PCICR |= (1 << PCIE0);   // enable Pin change interrupt group 0
    PCMSK0 |= (1 << PCINT1); // Unmask PCINT1, will trigger PCINT0_vect on rising or falling

    TCCR1A = 0; // Clear timer1 control registers to known state
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);  // Clear timer on compare, resets counter to o at OCR1A match
    TCCR1B |= (1 << CS10);   // no prescale (16 MHz -> 1 tick = 62.5 ns)
    OCR1A = 64000;           // 64000 ticks, 4 ms per compare match
    TIMSK1 |= (1 << OCIE1A); // enable Compare A interrupt

    uart_init(9600); // Begin serial at 9600 baud

   
    // LED on D13 (on the arduino)
    DDRB |= (1 << DDB5);     // D13 as output

    // Button on D2 (PD2 / INT0), enable pull-up, trigger on falling edge
    DDRD &= ~(1 << DDD2);    // D2 input
    PORTD |= (1 << PD2);     // pull-up enabled
    EICRA |= (1 << ISC01);   // selects falling edge trigger
    EIMSK |= (1 << INT0);    // allows the external interrupt to fire ISR(INT0_vect) when the edge occurs

    // ADC on A0 (ADC0), AVcc reference, prescaler /128 (125 kHz ADC clock)
    ADMUX  = (1 << REFS0);                              // AVcc (5V) ref, A0 channel
    ADCSRA = (1 << ADEN) | (1 << ADIE) |                // enable ADC + ADC interrupt
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);// /128, ADC clock = 125 kHz
    ADCSRA |= (1 << ADSC);                              // start first conversion

    sei();           // Set Enable Interupts

    for (;;){ // inf loop

        unsigned long freq; // Frequency in Hz
        unsigned long duty; // Duty cycle in %

        if (T_period > 0)
        { // When we have at least one full cycle measured
            // Frequency = 1,000,000 / period_in_microseconds
            freq = 1000000UL / T_period;

            // Duty cycle = 100 * (time_high / period)
            duty = (100UL * T_high) / T_period;
        }
        else
        {
            // If period is zero (no signal yet), avoid division by zero
            freq = 0;
            duty = 0;
        }

        uart_puts("Duty: ");
        uart_putu32(duty); // Prints Dutycykle and Freq in serial monitor
        uart_puts("%  Freq: ");
        uart_putu32(freq);
        uart_puts(" Hz\r\n");



        uart_puts("ADC A0 raw: ");
        uart_putu32(adc_raw);
        uart_puts("  ModeFlag: ");
        uart_putu32(mode_flag);
        uart_puts("\r\n");



    }
}
