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
DDRB |= 0b00011111; // PIN 8-13 as output
}

void output_set(unsigned char value){
if(value==0) PORTB &= 0xfe; else PORTB |= 0x01;
}

void input_init(void){
DDRD |= 0x7c;  // PIN 2-6 as input (Bouton Joystick + boutons)
DDRC |= 0x03;		// PIN 0-1 analogiques comme input (x et y joystick analogique)
}

unsigned char input_get(void){
return ((PIND&0x04)!=0)?1:0;
}

void commande_leds(){
	unsigned char temp_serial, leds;
	temp_serial = get_serial();
			if (temp_serial !=0){
				leds = temp_serial;
				if (leds == 65) PORTB = PORTB | 0x01; 
				if (leds == 66) PORTB = PORTB | 0x02;
				if (leds == 67) PORTB = PORTB | 0x04;
				if (leds == 68) PORTB = PORTB | 0x08;
				if (leds == 69) PORTB = PORTB | 0x10;
				if (leds == 70) PORTB = PORTB | 0x20;
				
				if (leds == 97) PORTB = PORTB & 0xfe;
				if (leds == 98) PORTB = PORTB & 0xfd;
				if (leds == 99) PORTB = PORT B & 0xfb;
				if (leds == 100) PORTB = PORT B & 0xf7;
				if (leds == 101) PORTB = PORT B & 0xef;
				if (leds == 102) PORTB = PORT B & 0xdf;
			}
}

// Dummy main

int main(void){
    init_serial(9600);
    output_init();
    input_init();
    
    unsigned char boutons, boutons_anc;
    unsigned char joystick_x, joystick_x_anc;
    unsigned char joystick_y, joystick_y_anc;

    
    boutons_anc = PORTD & 0x7C;
    ad_init(0);
    joystick_x_anc = ad_sample();
    ad_init(1);
    joystick_y_anc = ad_sample();
    
    while(1){
    	boutons = PORTD & 0x7C;	
    	ad_init(1);
    	joystick_x = ad_sample();
    	ad_init(0);
    	joystick_y = ad_sample();
    
    	if (boutons_anc != boutons) send_serial(boutons);
    	if ((joystick_x_anc != joystick_x) || (joystick_y_anc != joystick_y){
    	  send_serial(joystick_x);
    	  send_serial(joystick_y);
			}
			
			commande_leds();
    }
    
	return 0;
}












