/*	Author :Josh Madrid (Jmadr004@ucr.edu)
 *	Lab Section: 022
 *	Assignment: Final Project Slave Code
 *	Exercise Description: Created: 10/31/2017
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "bit.h"

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


unsigned char temp;
unsigned char hold_press;
unsigned char press_temp;
unsigned char GD_flag=0x00;
unsigned char temp=0x00;
unsigned char foward[8]={0x01,0x03,0x02,0x06,0x04,0x0C,0x08,0x09};
unsigned char backward[8]={0x08,0x0C,0x04,0x06,0x02,0x03,0x01,0x09};
unsigned char test=0x01;

unsigned char GetKeypadKey() {
	
	PORTA = 0xEF; // Enable col 4 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('1'); }
	if (GetBit(PINA,1)==0) { return('2'); }
	if (GetBit(PINA,2)==0) { return('3'); }
	if (GetBit(PINA,3)==0) { return('A'); }
	
	// Check keys in col 2
	PORTA = 0xDF; // Enable col 5 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('4'); }
	if (GetBit(PINA,1)==0) { return('5'); }
	if (GetBit(PINA,2)==0) { return('6'); }
	if (GetBit(PINA,3)==0) { return('B'); }
	
	
	// Check keys in col 3
	PORTA = 0xBF; // Enable col 6 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('7'); }
	if (GetBit(PINA,1)==0) { return('8'); }
	if (GetBit(PINA,2)==0) { return('9'); }
	if (GetBit(PINA,3)==0) { return('C'); }
	
	
	// Check keys in col 4
	PORTA = 0x7F;// Enable col 6 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('*'); }
	if (GetBit(PINA,1)==0) { return('0'); }
	if (GetBit(PINA,2)==0) { return('#'); }
	if (GetBit(PINA,3)==0) { return('D'); }
	
	
	return('\0'); // default value
	
}

enum Key_States{KS_I, KS_W, KS_A, KS_A1, KS_A2, KS_A3, KS_Open_Close}Key_State;
void Keypad_Tick()
{
	switch(Key_State)
	{
		case KS_I:
			Key_State=KS_W;
		break;

		case KS_W:
			if(hold_press=='#')
			{
				Key_State=KS_A;
				PORTB = 0x01;
			}
			else
			{
				Key_State=KS_W;
				PORTB=0x00;
			}
		break;

		case KS_A:
			if(hold_press=='#')
			{
				Key_State=KS_A;
			}
			else if(hold_press=='1')
			{
				PORTB = 0x02;
				Key_State=KS_A1;
				
			}
			else
			{
				Key_State=KS_W;
				PORTB=0x00;
			} 
		break;

		case KS_A1:
			 if(hold_press=='1')
			 {
				 Key_State=KS_A1;
			 }
			else if(hold_press=='2')
			{
				PORTB = 0x04;
				Key_State=KS_A2;
				
			}
			else
			{
				Key_State=KS_W;
				PORTB=0x00;
			}
		break;

		case KS_A2:
			 if(hold_press=='2')
			 {
				 Key_State=KS_A2;
			 }
			else if(hold_press=='3')
			{
				PORTB=0x08;
				Key_State=KS_A3;	
			}
			else
			{
				Key_State=KS_W;
				PORTB=0x00;
			}
		break;

		case KS_A3:
			if(hold_press=='3')
			{
				Key_State=KS_A3;
			}
			else if(hold_press=='A')
			{
				PORTB=0x10;
				Key_State=KS_Open_Close;	
			}
			else
			{
				Key_State=KS_W;
				PORTB=0x00;
			}
		break;

		case KS_Open_Close:
			
			if(GD_flag==0x00)
			{
				PORTB=0xFF;
				GD_flag = 0x01;
				Key_State=KS_W;
			}
			else if(GD_flag==0x03)
			{
				PORTB=0xFF;
				GD_flag = 0x04;
				Key_State=KS_W;
			}
			else
			{
				Key_State=KS_W;
			}
			
		break;

		default:
			Key_State=KS_I;
		break;
	}
	
};


unsigned char count=0x00;
unsigned short counter=0x00;
enum GD_STATES{GD_I, GD_W, GD_O, GD_C}GD_STATE;
void GD_TICK()
{
	switch(GD_STATE)
	{
		case GD_I:
		GD_STATE=GD_W;
		break;
		
		case GD_W:
		if(GD_flag == 0x01)
		{
			GD_flag = 0x03;
			PORTB=0x01;
			GD_STATE=GD_O;	
		}
		else if(GD_flag == 0x04)
		{
			GD_flag = 0x00;
			GD_STATE=GD_C;
		}
		else
		{
			PORTB = 0x00;
			GD_STATE=GD_W;
		}
		break;




		case GD_O:
		if(counter<4096)
		{
			//PORTB=0x02;
			if(count >= 8){count=0;}
			PORTC=foward[count];
			count++;
			counter++;
			GD_STATE=GD_O;
		}
		else
		{
			count=0;
			counter=0;	
			GD_STATE=GD_W;
		}
		break;

		case GD_C:
		if(counter<4096)
		{
			//PORTB=0x02;
			if(count >= 8){count=0;}
			PORTC=backward[count];
			count++;
			counter++;
			GD_STATE=GD_C;
		}
		else
		{
			count=0;
			counter=0;
			GD_STATE=GD_W;
		}
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
		break;


		case GD_O:
		break;

		case GD_C:
		break;

		default:
		GD_STATE=GD_I;
		break;
	}

}

enum TR_STATES{TR_I, TR_W}TR_STATE;
void TR_TICK()
{

	switch(TR_STATE)
	{

		case TR_I:
			TR_STATE=TR_W;
		break;

		case TR_W:
			
			if (USART_HasReceived(0))
			{
				//...receive data...
				temp = USART_Receive(0);			 // write data received by USART1 to temp
			}

			if(temp==0x01)
			{
				GD_flag = 0x01;
			}
			else if(temp == 0x04)
			{
				GD_flag = 0x04;
			}
			else
			{
				temp = 0x00;
			}

						
			if(USART_IsSendReady(0))
			{
				USART_Send(GD_flag,0);
				if (USART_HasTransmitted(0))
				{
					
				}
				else
				{
					
				}
			}

			TR_STATE=TR_W;	
		break;

		default:
			TR_STATE=TR_I;
		break;

	}

}


int main(void)
{
	DDRA = 0xF0; PORTA = 0x0F; 
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;

	GD_STATE=GD_I;
	Key_State=KS_I;
	TR_STATE=TR_I;

	TimerSet(3);
	TimerOn();

	initUSART(0);

    /* Replace with your application code */
    while (1) 
    {

		press_temp=GetKeypadKey();
		if(press_temp !='\0')
		{
			hold_press = press_temp;
		}

		Keypad_Tick();
		GD_TICK();
		TR_TICK();
		//PORTB=0x10;


		
		while(!TimerFlag);
		TimerFlag = 0;
		
		

		PORTA = temp;

    }
	}	


