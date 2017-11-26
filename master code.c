/*	Author :Josh Madrid (Jmadr004@ucr.edu)
 *	Lab Section: 022
 *	Assignment: Final Project Master Code
 *	Exercise Description: Created: 10/31/2017
 *	
 *	I acknowledge all content contained herein, excluding template,example and provided code,
 * is my own original work.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "usart.h"


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

void ADC_init() 
{
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}


unsigned char foward[8]={0x01,0x03,0x02,0x06,0x04,0x0C,0x08,0x09};
unsigned char backward[8]={0x08,0x0C,0x04,0x06,0x02,0x03,0x01,0x09};
unsigned char count=0;
unsigned long temp_counter=0x00;
unsigned short counter =0x00;
unsigned char FD_Open=0x00;
unsigned char FD_Close=0x00;
unsigned char GD_Open=0x00;
unsigned char GD_Close=0x00;
unsigned char GD_Flag=0x00;
unsigned char temp=0x00;
unsigned char test=0x01;
unsigned char light1=0x00;
unsigned char light2=0x00;
unsigned char light3=0x00;
unsigned char light4=0x00;
unsigned char hold_BLE =  0x20;
short temperature;
unsigned char temp_check_flag=0x00;


//code provided by Carlos Antillana with his permission to use 
//https://github.com/carlossantillana/MIH/blob/master/main.c
const unsigned char celToFarArray[100] PROGMEM = {32, 33, 35, 37, 39, 41, 42, 44, 46, 48, 50, 51, 53, 55, 57, 59, 60, 62, 64, 66, 68, 69, 71, 73,
	75, 77, 78, 80, 82, 84, 86, 87, 89, 91, 93, 95, 96, 98, 100, 102, 104, 105, 107, 109, 111, 113, 114, 116, 118, 120, 122, 123, 125,
	127, 129, 131, 132, 134, 136, 138, 140, 141, 143, 145, 147, 149, 150, 152, 154, 156, 158, 159, 161, 163, 165, 167, 168, 170, 172,
174, 176, 177, 179, 181, 183, 185, 186, 188, 190, 192, 194, 195, 197, 199, 201, 203, 204, 206, 208, 210};


//code provided by Carlos Antillana with his permission to use
//https://github.com/carlossantillana/MIH/blob/master/main.c
inline unsigned char celToFar(unsigned char C);

enum FD_STATES{FD_I, FD_W, FD_O, FD_C}FD_STATE;
void FD_TICK()
{


	switch(FD_STATE)
	{
		case FD_I:
			FD_STATE=FD_W;
		break;
			
		case FD_W:
			if(FD_Open == 0x01 && FD_Close == 0x00)
			{
				FD_STATE=FD_O;
			}
			else if(FD_Open == 0x00 && FD_Close == 0x01)
			{
				FD_STATE=FD_C;
			}
			else
			{
				FD_STATE=FD_W;
			}
		break;


		case FD_O:
			if(counter<4096)
			{
				if(count >= 8){count=0;}
				PORTC=foward[count];
				count++;
				counter++;
				FD_STATE=FD_O;
			}
			else
			{
				count=0;
				counter=0;
				FD_Open = 0x00;
				FD_STATE=FD_W;
			}
		break;

		case FD_C:
		if(counter<4096)
		{
			if(count >= 8){count=0;}
			PORTC=backward[count];
			count++;
			counter++;
			FD_STATE=FD_C;
		}
		else
		{
			count=0;
			counter=0;
			FD_Close = 0x00;
			FD_STATE=FD_W;
		}
		break;

		default:
			FD_STATE=FD_I;
		break;
	}

	switch(FD_STATE)
	{
		case FD_I:
		FD_STATE=FD_W;
		break;
		
		case FD_W:
		break;


		case FD_O:
		break;

		case FD_C:
		break;

		default:
		FD_STATE=FD_I;
		break;
	}

}

enum GD_STATES{GD_I, GD_W, GD_O, GD_C}GD_STATE;
void GD_TICK()
{

	switch(GD_STATE)
	{
		case GD_I:
		GD_STATE=GD_W;
		break;
		
		case GD_W:
		if(GD_Open==0x01 && GD_Close==0x00)
		{
			GD_STATE=GD_O;
		}
		else if(GD_Open==0x01 && GD_Close==0x01)
		{
			GD_STATE=GD_C;
		}
		else
		{
			GD_STATE=GD_W;
		}
		break;

		case GD_O:
			
			GD_Open==0x00;
			GD_Flag=0x01;
			if(USART_IsSendReady(0))
			{
				USART_Send(GD_Flag,0);
			}			
			GD_STATE=GD_W;
		break;

		case GD_C:
			GD_Close==0x00;
			GD_Flag=0x04;

		break;

		default:
		GD_STATE=GD_I;
		break;
	}

	switch(GD_STATE)
	{
		case GD_I:
		GD_STATE=GD_W;
		break;
		
		case GD_W:
		GD_STATE=GD_W;
		break;


		case GD_O:
		GD_STATE=GD_W;
		break;

		case GD_C:
		GD_STATE=GD_W;
		break;

		default:
		GD_STATE=GD_I;
		break;
	}

}


enum TR0_STATES{TR0_I, TR0_W}TR0_STATE;
void TR0_TICK()
{

	switch(TR0_STATE)
	{

		case TR0_I:
		TR0_STATE=TR0_W;
		break;

		case TR0_W:	
		if(GD_Flag==0x01 || GD_Flag==0x04)
		{
			if(USART_IsSendReady(0))
			{
				USART_Send(GD_Flag,0);
			}
			

			GD_Flag=0x00;
		}
		TR0_STATE=TR0_W;
		break;

		default:
		TR0_STATE=TR0_I;
		break;

	}

}


enum TR1_STATES{TR1_I, TR1_W}TR1_STATE;
void TR1_TICK()
{

	switch(TR1_STATE)
	{

		case TR1_I:
		TR1_STATE=TR1_W;
		break;

		case TR1_W:
			if(temp_check_flag==0x00)
			{
				
			}
			else
			{
				if(USART_IsSendReady(1))
				{
					USART_Send(temperature,1);
					if (USART_HasTransmitted(1))
					{
						//PORTA=0x01;
					}
					else
					{
						//PORTA=0x00;
					}
				}
				temp_check_flag=0x00;
			}
			TR1_STATE=TR1_W;
		break;

		default:
		TR1_STATE=TR1_I;
		break;

	}

}


enum RX1_STATES{RX1_I, RX1_W, RX1_0, RX1_1,RX1_2,RX1_3,RX1_4,RX1_5,RX1_6,RX1_7,RX1_8,RX1_9}RX1_STATE;
void RX1_TICK()
{
	//transitions
	switch(RX1_STATE)
	{
		case RX1_I:
			RX1_STATE=RX1_W;
		break;

		case RX1_W:
			if (USART_HasReceived(1))
			{
				//...receive data...
				temp = USART_Receive(1);			 // write data received by USART1 to temp
			
				hold_BLE = temp - 48;
			}

			if(hold_BLE==0)
			{
				RX1_STATE=RX1_0;
			}
			else if(hold_BLE==1)
			{
				RX1_STATE=RX1_1;
			}
			else if(hold_BLE==2)
			{
				RX1_STATE=RX1_2;
			}
			else if(hold_BLE==3)
			{
				RX1_STATE=RX1_3;
			}
			else if(hold_BLE==4)
			{
				RX1_STATE=RX1_4;
			}
			else if(hold_BLE==5)
			{
				RX1_STATE=RX1_5;
			}
			else if(hold_BLE==6)
			{
				RX1_STATE=RX1_6;
			}
			else if(hold_BLE==7)
			{
				RX1_STATE=RX1_7;
			}
			else if(hold_BLE==8)
			{
				RX1_STATE=RX1_8;
			}
			else if(hold_BLE==9)
			{
				RX1_STATE=RX1_9;
			}
			else
			{
				RX1_STATE=RX1_W;
			}
		break;

		case RX1_0:
			RX1_STATE=RX1_0;
		break;

		case RX1_1:
			RX1_STATE=RX1_1;
		break;

		case RX1_2:
			RX1_STATE=RX1_2;
		break;

		case RX1_3:
			RX1_STATE=RX1_3;
		break;

		case RX1_4:
			RX1_STATE=RX1_4;
		break;

		case RX1_5:
			RX1_STATE=RX1_5;
		break;

		case RX1_6:
			RX1_STATE=RX1_6;
		break;

		case RX1_7:
			RX1_STATE=RX1_7;
		break;

		case RX1_8:
			RX1_STATE=RX1_8;
		break;

		case RX1_9:
			RX1_STATE=RX1_9;
		break;

		default:
			RX1_STATE=RX1_I;
		break;
	}

	//actions
	switch(RX1_STATE)
	{
		case RX1_I:
		break;

		case RX1_W:
		break;

		case RX1_0:
			temp_check_flag = 0x01;
			hold_BLE = 0x20;
			RX1_STATE=RX1_W;
		break;

		case RX1_1:
			FD_Open = 0x01;
			hold_BLE = 0x20;
			RX1_STATE=RX1_W;
		break;

		case RX1_2:
			FD_Close = 0x01;
			hold_BLE = 0x20;
			RX1_STATE=RX1_W;
		break;

		case RX1_3:
			GD_Flag=0x01;
			hold_BLE = 0x20;
			RX1_STATE=RX1_W;
		break;

		case RX1_4:
			GD_Flag=0x04;
			hold_BLE = 0x20;
			RX1_STATE=RX1_W;
		break;

		case RX1_5:
			if(light1==0x00)
			{
				PORTB = SetBit(PORTB,0,1);
				light1=0x01;
			}
			else if(light1==0x01)
			{
				PORTB = SetBit(PORTB,0,0);
				light1=0x00;
			}
			hold_BLE = 0x20;
			RX1_STATE=RX1_W;
		break;

		case RX1_6:
			if(light2==0x00)
			{
				PORTB = SetBit(PORTB,1,1);
				light2=0x01;
			}
			else if(light2==0x01)
			{
				PORTB = SetBit(PORTB,1,0);
				light2=0x00;
			}
			hold_BLE = 0x20;
			RX1_STATE=RX1_W;
		break;

		case RX1_7:
			if(light3==0x00)
			{
				PORTB = SetBit(PORTB,2,1);
				light3=0x01;
			}
			else if(light3==0x01)
			{
				PORTB = SetBit(PORTB,2,0);
				light3=0x00;
			}
			hold_BLE = 0x20;
			RX1_STATE=RX1_W;
		break;

		case RX1_8:
			if(light4==0x00)
			{
				PORTB = SetBit(PORTB,3,1);
				light4=0x01;
			}
			else if(light4==0x01)
			{
				PORTB = SetBit(PORTB,3,0);
				light4=0x00;
			}
			hold_BLE = 0x20;
			RX1_STATE=RX1_W;
		break;

		case RX1_9:
		break;

		default:
		RX1_STATE=RX1_I;
		break;
	}





};



void TMP_TICK()
{


		//code provided by Carlos Antillana with his permission to use
		//https://github.com/carlossantillana/MIH/blob/master/main.c
		unsigned short voltage=ADC;
		unsigned short tmpVoltage=voltage;
		voltage =  (tmpVoltage >> 2) + (tmpVoltage << 2);// first divide by 4 to get .25 * volt then add that value by 4 * volt
		temperature = (voltage - 500) / 10;//turn signal to C
		temperature = celToFar(temperature);
		
	
	
	if(temp_counter<=5000)
	{
		temp_counter++;
	}
	else
	{
		if(temperature >= 80)
		{
			PORTA = SetBit(PORTA,1,1);

		}
		else
		{
			PORTA = SetBit(PORTA,1,0);
			
		}
		
		temp_counter=0x00;

	}
	


}

int main(void)
{
	DDRA=0x00; PORTA=0xFF;
	DDRB=0xFF; PORTB=0x00;
	DDRC=0xFF; PORTC=0x00;

	FD_STATE=FD_I;
	TR1_STATE=TR1_I;
	
	initUSART(0);
	initUSART(1);

	TimerSet(3);
	TimerOn();

	ADC_init();

	/* Replace with your application code */
	while (1)
	{
		RX1_TICK();
		FD_TICK();
		//GD_TICK();
		TR0_TICK();
		TMP_TICK();
		TR1_TICK();
		while(!TimerFlag);
		TimerFlag = 0;
	}
}

//code provided by Carlos Antillana with his permission to use
//https://github.com/carlossantillana/MIH/blob/master/main.c
inline unsigned char celToFar(unsigned char C){
	return pgm_read_word(&celToFarArray[C]);
}
