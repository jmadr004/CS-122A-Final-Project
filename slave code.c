#ifndef	F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b)
{
	return (b ? (x|(0x01<<k)) :(x &~(0x01<<k)) );
			//set bit to 1		//set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k)
{ return ((x & (0x01 << k)) != 0);}

volatile unsigned char TimerFlag = 0; //TimerISR() sets this to 1. c programmer should clear to 0

//Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; //start count from here down to 0. default 1ms.
unsigned long _avr_timer_cntcurr =0; //current internal count of 1ms ticks


void TimerOn()
{
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit3 = 0; CTC mode (clear timer on compare)
	//bit2bit1bit0=011: pre-scaler /64
	//00001011: 0x0B
	//SO, 8 MHz clock or 8,000,000 / 64 = 125,000 ticks/s
	//Thus, TCNT1 register will count at 125,000 ticks/s

	//AVR output compare register OCR1A
	OCR1A = 125; // timer interrupt will be generated when TCTN1 == OCR1A
	//We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125;
	//so when TCHTN1 register equals 125,
	//1 ms has passed. Thus, we compare to 125
	//AVR timer interrupt mask register
	TIMSK1 = 0x02; //bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1 =0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr milliseconds

	//enable global interrupts
	SREG |= 0x80;// 0x80 : 1000000

}

void TimerOFF()
{
	TCCR1B = 0x00; //bit3bit1bit0: timer off
}
void TimerISR()
{
	TimerFlag = 1;
}

//in our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect)
{
	//CPU automattically calls when TCNT1==OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; //Count down to 0 rather than up to TOP
	if(_avr_timer_cntcurr==0)//results in a more efficient compare
	{
		TimerISR(); //call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;

}

unsigned char counter=0;
unsigned char array1[8]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
unsigned char array2[8]={0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01};
unsigned char array3[8]={0xFF,0xAA,0xBB,0x11,0xEE,0x22,0xEE,0x33};
unsigned char button1=0x00;
unsigned char button2=0x00;
unsigned char pass_data=0;

// Ensure DDRC is setup as output
void transmit_data(unsigned char data) 
{

		//prepare for transmission
		PORTC = SetBit(PORTC,3,1);//SRCLR
		_delay_ms(10);// delay
		PORTC = SetBit(PORTC,1,0); // RCLK
		_delay_ms(10);// delay
		
		//transmit
		PORTC = SetBit(PORTC,2,0); // SRCLk
		for(int i = 0; i<8;i++)
		{
			PORTC = SetBit(PORTC,2,0); // SRCLk
			PORTC = SetBit(PORTC,0,GetBit(data,i)); //SER
			PORTC = SetBit(PORTC,2,1); // SRCLk
		}
		PORTC = SetBit(PORTC,2,1); // SRCLk
		_delay_ms(10);// delay
		PORTC = SetBit(PORTC,1,1); // RCLK
		_delay_ms(10);// delay
		PORTC = SetBit(PORTC,3,0);//SRCLR
		_delay_ms(10);// delay

}

enum LSTATES{LS_I,LS_W,LS_A1,LS_A2,LS_A3,LS_OFF}LSTATE;

void Light_tick()
{
	switch(LSTATE)
	{
		case LS_I:
			LSTATE=LS_W;
		break;

		case LS_W:
			if(button1&&button2)
			{
				LSTATE=LS_OFF;
			}
		LSTATE=LS_W;
		break;
		
		case LS_A1:
		break;
		
		case LS_A2:
		break;
		
		case LS_A3:
		break;

		case LS_OFF:
			if(button1&&button2)
			{
				LSTATE=LS_W;
			}
			else
			{
				LSTATE=LS_OFF;
			}
		break;


	}		
};


int main(void)
{
	DDRC=0xFF; PORTC=0x00; 
	DDRB=0x00; PORTB=0x0F;
	TimerSet(50);
	TimerOn();
    /* Replace with your application code */


    while (1) 
    {
		button1 = ~PINB & 0x01;
		button2 = ~PINB & 0x02;

		if(button1)
		{	
			if(pass_data==0xFF)
			{

			}
			else
			{
				pass_data++;
			}
		}
		else if(button2)
		{

			if(pass_data==0x00)
			{

			}
			else
			{
				pass_data--;
			}
			
		}
		transmit_data(pass_data);

	 	while(!TimerFlag);
	 	TimerFlag = 0;
    }

}

