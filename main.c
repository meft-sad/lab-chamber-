#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

/* define baud rate as BAUD before */
#define BAUD 115200
#define UBRRVAL F_CPU/8/BAUD-1


//Variables fot the distance
volatile uint16_t Capt1, Capt2; //VARIABLES TO HOLD TIMESTAMPS
volatile uint8_t flag = 0; //CAPTURE FLAG
volatile uint32_t speed_sound = 0; //Speed of sound in cm/s
uint32_t dist;
uint32_t dist2;
int j = 0;

//Pin numbers for the distance
const int trigPin = PB1;
const int echoPin = PB0;
const int led_d = PD3;

//Pin numbers for the flame
const int sensorPin = PD2; // select the input pin for the LDR
const int led_f = PD4;

// Variables for the temperature
int look_upV[]= {250, 275, 300, 325,350, 375, 400, 425, 450, 475,500, 525, 550, 575,600, 625, 650, 675,700,725,750,775,784,825,850,875,900,925}; //Array for the voltage obtained by the ADC of Arduino
double look_upT[] = {1.04472169 , 3.591354708 , 6.032690396 , 8.39210897 , 10.68870604 , 12.93856812 , 15.15566414 , 17.35249651 , 19.54059969 , 21.73094314 , 23.93427818 , 26.16145787 , 28.42375479 , 30.73319991 , 33.1029684 , 35.54784356 , 38.08480115 , 40.73377405 , 43.51868767 , 46.46890668 , 49.62132143 , 53.02346002 , 54.32087756 , 60.85209899 , 65.48760291 , 70.82825543 , 77.1657953 , 85.00520735}; //Array for the corresponding temperature
int i = 0;
int n = 0;
int v_small = 0;
int v_big = 0;
double t_small = 0.0;
double t_big = 0.0;
int analogResult = 0; //Variable for the result of the analog to digital conversion of the ADC
double temp = 0.0; //Variable for the calcultation of temperature inside the function
double T = 0.0; //Variable for temperature in the main

//Pin for the temperature: control LED, it turns on if the temperature is too high
const int led_t = PD6;

//LED THAT CONTROLS THAT TURNS ON IF ALL THE CONTROLS ARE OK
const int led_green = PB2;

//Button variables: if the button was pressed and everything was ok the status becomes 1
//If something went wrong (flame detected or temperature too high or something too close) the status turns 0 
//and the greeen led cannot become high again even if everything is ok if the button is not pressed again. 

const int button = PB4;
volatile int status = 0;

//Checks variables
volatile int check_d = 0;
volatile int check_f = 0;
volatile int chechk_t = 0;

//UART
static int uputc(char,FILE*);

static int uputc(char c,FILE *stream)
{
    if (c == '\n')
	uputc('\r',stream);
	loop_until_bit_is_set(UCSR0A, UDRE0);	/* wait until we can send a new byte */
	UDR0 = (uint8_t) c;
	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uputc, NULL,_FDEV_SETUP_WRITE);

//uart receive isr
ISR(USART_RX_vect)
{
	uint8_t c = UDR0;
	putchar(c);
}

/*INTERRUPTS FOR DISTANCE*/

void inter_set_distance(void){
  TCCR1A = 0; //Disable compare interrupts
  TCCR1B = 0;
  TCNT1=0; //SETTING INTIAL TIMER VALUE

  TCCR1B|=(1<<ICES1); //SETTING FIRST CAPTURE ON RISING EDGE
  TIMSK1|=(1<<ICIE1); //ENABLING INPUT CAPTURE
  TIFR1 &= ~(1 << ICF1); //Clear the flag
  TCCR1B|=(1<<CS10);
  TCCR1B &= ~(1<<CS12);
  TCCR1B &= ~(1<<CS11);//STARTING TIMER WITH NO PRESCALER

  sei(); //ENABLING GLOBAL INTERRUPTS

  }

//Calculation of temperature given the voltage
double calc_temp(int RawADC){
  for(i = 0 ; i < n ; i++){
    if( RawADC < look_upV[i]){
       v_big = look_upV[i];
       v_small = look_upV[i-1];
       t_big = look_upT[i];
       t_small = look_upT[i-1];
       temp = t_small + ((t_big-t_small)/(v_big-v_small))*(RawADC-v_small);
       break;
      }
    else if ( RawADC == look_upV[i]){
       temp = look_upT[1];
        }
    }

  return temp;
  }

//Configuration of the ADC

void analog_init(void){

   ADCSRA = 0;             // clear ADCSRA register
   ADCSRB = 0;             // clear ADCSRB register
   ADMUX |= (1 << MUX0);    // set A1 analog input pin

   //ADMUX &= ~((1<<REFS1)|(1<<REFS0));  //Use Vcc as reference
   ADMUX |= (1 << REFS0);
   ADMUX &= ~(1 << ADLAR); //right align ADC value to 8 bits from ADCH register
   // ADCSRA |= (1 << ADPS2)| (1 << ADPS1)| (1 << ADPS0); //128 prescalare
   ADCSRA |= (1 << ADPS2); //Set prescalar of 16

   //Free running mode
   ADCSRB &= ~ ((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0));

   //Enable ADC interrupts
   ADCSRA |= (1<<ADIE);
   sei();

   // Enable auto trigger
   ADCSRA |= (1 << ADATE);
   //Enable ADC
   ADCSRA |= (1<<ADEN);
   //Start conversion
   ADCSRA|=(1<<ADSC);
}


int main(void)
{
	
  stdout=&mystdout;

  /* uart config */
  UCSR0A = (1 << U2X0); /* this sets U2X0 to 1 */
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  UCSR0C = (3 << UCSZ00); /* this sets UCSZ00 to 3 */

  /* baudrate setings (variable set by macros) */
  UBRR0H = (UBRRVAL) >> 8;
  UBRR0L = UBRRVAL;

  //SET THE PINS FOR THE DISTANCE

  //Set echopin as input and enable the pull-up
  DDRB &= ~(1 << echoPin); //Input
  PORTB |= (1 << echoPin); //Enable pull-up resistors

  //Set trigpin as output
  DDRB |= (1 << trigPin); //Output
  PORTB &= ~(1 << trigPin);  //Set the output to zero

  //Set led as output
  DDRD |= (1 << led_d);
  PORTD &= ~(1 << led_d);

  //SET THE PINS FOR THE FLAME
  // declare the ledPin as an OUTPUT:
  DDRD |= (1 << led_f);
  //Declare the pin sensor as input
  DDRD &= ~(1 << sensorPin);
  PORTD |=(1 << sensorPin);
  //SET THE PINS FOR THE TEMPERATURE
  DDRD |= (1 << led_t);

  //Green LED as output
  DDRB |= (1 << led_green);

  //Button pin as input
  DDRB &= ~(1 << button);
  PORTB |=(1 << button);
	
  n = sizeof(look_upV)/sizeof(look_upV[0]);

  //RECALL THE FUNCTIONS
  analog_init();
  inter_set_distance();

  //Send the first signal for the distance
  PORTB |= (1 << trigPin);
  _delay_us(10);
  PORTB &= ~(1 << trigPin);

  while(1) {
    //Control for the flame
     if ((PIND & (1<<PD2))){
       puts("Flame!!\n");
       PORTD |= (1 << led_f);
       check_f = 0;
     }
     else {
       PORTD &= ~(1 << led_f);
       check_f = 1;
     }
    //TEMPERATURE CHECK
    T =  calc_temp(analogResult);
    if( T > 40){
	puts("Temperature too high!");
	PORTD |= (1 << led_t);
	chechk_t = 0;
     }
    else {
        PORTD &= ~(1 << led_t);
	chechk_t = 1;
     }


    //Calculate the distance
     if (flag == 2)
     {

       speed_sound = 100*(331.45 + (0.62 * T)); //Model for the speed of sound as a function of temperature in cm/s

       dist =(Capt2 - Capt1)*speed_sound/2;
       dist2 = dist/F_CPU;

       if (dist2 < 10){
         PORTD |= (1 << led_d);
         puts("Too close!\n");
	 check_d = 0;
        }
       else {
         PORTD &= ~(1 << led_d);
	 check_d = 1;
       }

	     

       //Control if the button was pressed
       if(!(PINB & (1 << button))){
	  status = 1;
       }
       
       //Control that everything is fine: distance, temperature and flame all in the limits and the button was pressed so that status is 1
       if((check_f & check_d & chechk_t) & status){
	  PORTB |= (1 << led_green);
	  puts("EVERYTHING IS FINE!\n");
	}
       else{
	  //Here something went wrong so the led turns off and the status is set to 0. 
	  //In this way when everything is ok again the led cannot turn on if before the button is not pressed.
	  status = 0;
	  PORTB  &= ~(1 << led_green);
        }


       ADCSRA|=(1<<ADSC);//start next conversion

       flag = 0; //CLEARING FLAGS

       TIMSK1|=(1<<ICIE1); //ENABLING INPUT CAPTURE INTERRUPTS
       TCNT1 = 0; //Clear the counter

       //Send an input at the end of the loop only if the previous input was received that means that flag == 2
       PORTB |= (1 << trigPin);
       _delay_us(10);
       PORTB &= ~(1 << trigPin);
       TCNT1=0;
     }

  }
}

//Interruption routine for the distance
ISR(TIMER1_CAPT_vect){
	
    //PORTD |= (1 << led);
    if (flag == 0){
	 Capt1 = ICR1; //SAVING CAPTURED TIMESTAMP
	 TCCR1B &=~(1<<ICES1); //CHANGE CAPTURE ON FALLING EDGE
     }
    if (flag == 1){
	    
	 Capt2 = ICR1; //SAVING CAPTURED TIMESTAMP
	 TCCR1B|=(1<<ICES1); //CHANGING CAPTURE ON RISING EDGE

	 TIFR1 &= ~(1 << ICF1); //Clear the flag
     }
     flag = flag + 1;
}


//Interrupts for the ADC
ISR(ADC_vect){
  analogResult = ADC; //Write in this variable the value of the conversion
  }
