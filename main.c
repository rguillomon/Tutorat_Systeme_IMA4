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
DDRB |= 0b00111111; // PIN 8-13 as output
}

void output_set(unsigned char value){
if(value==0) PORTB &= 0xfe; else PORTB |= 0x01;
}

void input_init(void){
DDRD |= 0b01111100;  // PIN 2-6 as input (Bouton Joystick + boutons)
DDRC |= 0x03;		// PIN 0-1 analogiques comme input (x et y joystick analogique)
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
int get_buttons(void){
	unsigned boutons = ((PIND & 0x7C)>>2) | 0b00100000;
	return(boutons);
}

/* Met en forme l'octet de l'axe pos du joystick */
int shape_joy(unsigned char pos){
	pos = (pos >> 3) | 0b00100000;
	return(pos);
}

/* Récupération de la valeur de l'axe du joystick sur la chaîne channel du CAN */
int get_joystick(int channel){
	  unsigned char axis;
	  ad_init(channel);
    axis = ad_sample();
    shape_joy(axis);
    return(axis);
}

/* Dummy main */
int main(void){
  init_serial(9600);
  output_init();
  input_init();
  
  unsigned char boutons, boutons_anc;
  unsigned char joystick_x, joystick_x_anc;
  unsigned char joystick_y, joystick_y_anc;

	//Récupération des valeurs des boutons et joystick, et mise en forme
  boutons = get_buttons();
  joystick_x = get_joystick(1);
  joystick_y = get_joystick(0);
    
  //send_serial(boutons);
  
  while(1){
    
		boutons_anc = boutons;
		joystick_x_anc = joystick_x;
		joystick_y_anc = joystick_y;
    
   	//Récupération des valeurs des boutons et joystick, et mise en forme
  	boutons = get_buttons();
  	joystick_x = get_joystick(1);
  	joystick_y = get_joystick(0);
    	

    send_serial(boutons+2);
    //retour chariot
    send_serial(0x0a);
    send_serial(0x0d);
    	
    //Cas où aucun caractère n'est reçu
    if ((UCSR0A & (1<<RXC0)) == 0){
    	
    	//if (boutons_anc != boutons){
    	//if (joystick_y_anc != joystick_y){
    	//if ((boutons_anc != boutons) || (joystick_x_anc != joystick_x) || (joystick_y_anc != joystick_y)){
    		//send_serial(boutons);
      	//send_serial(joystick_y);
      	//send_serial(joystick_y);
      	
      	//send_serial('r');
    	  	
      	//retour chariot
      	//send_serial(0x0a);
      	//send_serial(0x0d);
			//}
		}
			
		else{
		//Cas où un caractère attent d'être reçu
		commande_leds();
		}
  }
 		  
	return 0;
}












