#define F_CPU 16000000UL   // CPU Clock speed
#include <avr/io.h>        // For register names
#include <avr/interrupt.h> // For interupts

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
    sei();           // Set Enable Interupts

    for (;;){ // inf loop

        unsigned long freq; // Frequency in Hz
        unsigned long duty; // Duty cycle in %

        if (T_period > 0){ // When we have at least one full cycle measured
            
            freq = 1000000UL / T_period;    // Frequency = 1,000,000 / period 

            
            duty = (100UL * T_high) / T_period; // Duty cycle = 100 * (time_high / period)
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
    }
}
