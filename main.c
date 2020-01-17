#include <avr/io.h>		// for the input/output register
#include <util/delay.h>

// For the serial port

#define CPU_FREQ        16000000L       // Assume a CPU frequency of 16Mhz
#define tempo						25
#define debit						9600						//débit liaison série en bauds 

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
DDRB |= 0b00111111; // PIN 8-13 as output (LED)
}

/*
void output_set(unsigned char value){
if(value==0) PORTB &= 0xfe; else PORTB |= 0x01;
}
*/

void input_init(void){
	DDRD &= 0b10000011;	// PIN 2-6 as input (Bouton Joystick + boutons)
	PORTD |= 0x7C; // Pull up de 0 à 1
}

/*
unsigned char input_get(void){
return ((PIND&0x04)!=0)?1:0;
}
*/

/* Commande des LED */
void commande_leds(){
	unsigned char temp_serial, leds;
	temp_serial = UDR0;
	if (temp_serial !=0){
		leds = temp_serial;
		// Allumer LED de 'A' à 'F'
		if (leds == 65) PORTB = PORTB | 0x01; 
		if (leds == 66) PORTB = PORTB | 0x02;
		if (leds == 67) PORTB = PORTB | 0x04;
		if (leds == 68) PORTB = PORTB | 0x08;
		if (leds == 69) PORTB = PORTB | 0x10;
		if (leds == 70) PORTB = PORTB | 0x20;
		
		// Eteindre LED de 'a' à 'f'
		if (leds == 97) PORTB = PORTB & 0xfe;
		if (leds == 98) PORTB = PORTB & 0xfd;
		if (leds == 99) PORTB = PORTB & 0xfb;
		if (leds == 100) PORTB = PORTB & 0xf7;
		if (leds == 101) PORTB = PORTB & 0xef;
		if (leds == 102) PORTB = PORTB & 0xdf;
	}
}

/* Récupération de la valeur des boutons et mise en forme */
unsigned char get_buttons(void){
	unsigned char boutons = PIND;
	boutons = boutons >>2;
	boutons = boutons & 0b00111111;
	boutons = boutons | 0b00100000;
	
	return boutons;
}

/* Met en forme l'octet de l'axe pos du joystick */
unsigned char shape_joy(unsigned char pos){
	pos = pos >>3;	//On récupère les 5 bits de poids forts de l'ADCH (diminuer sensibilité)
	pos = pos & 0b00111111;
	pos = pos | 0b00100000;
	return pos;
}

/* Récupération de la valeur de l'axe du joystick sur la chaîne channel du CAN */
unsigned char get_joystick(int channel){
	  unsigned char axis;
	  ad_init(channel);
    axis = ad_sample();
    axis = shape_joy(axis);
    return axis;
}

/* Dummy main */
int main(void){

	//INITIALISATIONS  
  unsigned char boutons, boutons_anc;
  unsigned char joystick_x=0x10, joystick_x_anc=0x10;
  unsigned char joystick_y=0x10, joystick_y_anc=0x10;
  
  init_serial(debit);
  output_init();
  input_init();

	//Récupération des valeurs des boutons et joystick, et mise en forme
  boutons = get_buttons();
  joystick_x = get_joystick(0);
  joystick_y = get_joystick(1);
  
  while(1){
		boutons_anc = boutons;
		joystick_x_anc = joystick_x;
		joystick_y_anc = joystick_y;
    
   	//Récupération des valeurs des boutons et joystick, et mise en forme
  	boutons = get_buttons();
  	joystick_x = get_joystick(0);
  	_delay_ms(tempo);
  	joystick_y = get_joystick(1);
    	
    //Port série libre
    if ((UCSR0A & (1<<RXC0)) == 0){
    	// Si une des grandeurs a changé
    	if ((boutons_anc != boutons) || (joystick_x_anc != joystick_x) || (joystick_y_anc != joystick_y)){
    		send_serial(boutons);	//on envoie l'état global des grandeurs
      	send_serial(joystick_x);
      	send_serial(joystick_y);    	  	
      	//retour chariot
      	send_serial(0x0a);
      	send_serial(0x0d);
			}
		}
		
		//Port série occupé	
		else{
		commande_leds();	//On gère la commande des leds
		}
  }  
	return 0;
}












