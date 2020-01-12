#include <avr/io.h>		// for the input/output register

// For the serial port

#define CPU_FREQ        16000000L       // Assume a CPU frequency of 16Mhz

void init_serial(int speed)
{
/* Set baud rate */
UBRR0 = CPU_FREQ/(((unsigned long int)speed)<<4)-1;

/* Enable transmitter & receiver */
UCSR0B = (1<<TXEN0 | 1<<RXEN0);

/* Set 8 bits character and 1 stop bit */
UCSR0C = (1<<UCSZ01 | 1<<UCSZ00);

/* Set off UART baud doubler */
UCSR0A &= ~(1 << U2X0);
}

void send_serial(unsigned char c)
{
loop_until_bit_is_set(UCSR0A, UDRE0);
UDR0 = c;
}

unsigned char get_serial(void) {
loop_until_bit_is_set(UCSR0A, RXC0);
return UDR0;
}

// For the AD converter

void ad_init(unsigned char channel)   
{   
ADCSRA|=(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);   
ADMUX|=(1<<REFS0)|(1<<ADLAR);
ADMUX=(ADMUX&0xf0)|channel;   
ADCSRA|=(1<<ADEN);
}   

//récupère la valeur de l'ADCH
unsigned int ad_sample(void){
ADCSRA|=(1<<ADSC);
while(bit_is_set(ADCSRA, ADSC));
return ADCH;
}

// For the I/O 
void output_init(void){
DDRB |= 0x01; // PIN 8 as output
}

void output_set(unsigned char value){
if(value==0) PORTB &= 0xfe; else PORTB |= 0x01;
}

void input_init(void){
DDRD &= 0xfb;  // PIN 2 as input
PORTD |= 0x04; // Pull-up activated on PIN 2
}

unsigned char input_get(void){
return ((PIND&0x04)!=0)?1:0;
}

// Dummy main

int main(void){
    init_serial(9600);
    ad_init(0); //pas sur que ce soit ici
    output_init();
    input_init();
    
    
return 0;
}
