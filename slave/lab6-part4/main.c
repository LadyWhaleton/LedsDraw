/*
 * lab6-part4.c
 *
 * Created: 10/19/2015 4:16:07 PM
 * Author : User1
 */ 

#include <avr/io.h>
#include "scheduler.h"
#include "SPI.h"

void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: Enables analog-to-digital conversion
	// ADSC: Starts analog-to-digital conversion
	// ADATE: Enables auto-triggering, allowing for constant
	//	    analog to digital conversions.
}

// Pins on PORTA are used as input for A2D conversion
//    The default channel is 0 (PA0)
// The value of pinNum determines the pin on PORTA
//    used for A2D conversion
// Valid values range between 0 and 7, where the value
//    represents the desired pin for A2D conversion
	void Set_A2D_Pin(unsigned char pinNum) {
		ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
		// Allow channel to stabilize
		static unsigned char i = 0;
		for ( i=0; i<15; i++ ) { asm("nop"); }
	}


unsigned int col = 0x80;	// LED pattern - 0: LED off; 1: LED on
unsigned int row = 0x01;  	// Row(s) displaying pattern.

enum MatrixStates {wait, display};
int Task_DisplayImage(int state) {
	// Transitions
	switch (state) {
		case wait:
			if (go)
				state = display;
				
			break;
		case display:
			if (x_axis == RIGHT && y_axis == UP && row != 0x01 && col != 0x01)
			{
				row = row >> 1;
				col = col >> 1;
			}
			else if (y_axis == UP && x_axis == LEFT && row != 0x01 && col != 0x80)
			{
				row = row >> 1;
				col = col << 1;
			}
			else if (x_axis == RIGHT && y_axis == DOWN && row != 16 && col != 0x01)
			{
				row = row << 1;
				col = col >> 1;
			}
			else if (x_axis == LEFT && y_axis == DOWN && row != 16 && col != 0x80)
			{
				row = row << 1;
				col = col << 1;
			}
			else if (y_axis == UP && row != 0x01)
				row = row >> 1;
			else if (y_axis == DOWN && row != 16)
				row = row << 1;
			else if (x_axis == LEFT && col != 0x80)
				col = col << 1;
			else if (x_axis == RIGHT && col != 0x01)
				col = col >> 1;
			
			state = wait;
			go = 0;
			x_axis = -1;
			y_axis = -1;
			
			break;
		default:
			state = wait;
			break;
	}

	
	PORTC = col;	// Pattern to display
	PORTD = ~(row);		// Row(s) displaying pattern
	
	return state;
}

// shift rows ~(0x01 << 1)

void initTasks()
{
	tasksNum = 2;
	task taskList[tasksNum];
	tasks = taskList;


	// define tasks
	unsigned char i=0; // task number

	// Task that will displays image on the LED matrix
	tasks[i].state = -1;
	tasks[i].period = 150;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Task_DisplayImage;
	++i;
	
	// Task that will obtain input from the keypad
	tasks[i].state = -1;
	tasks[i].period = 100;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Task_GetInput;
	++i;


	TimerSet(50); // value set should be GCD of all tasks
	TimerOn();

}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00; // LCD control lines, output
	DDRC = 0xFF; PORTC = 0x00; // LCD_data lines, output
	
	DDRD = 0xF0; PORTD = 0x0F;; // initialize input ports for keypad
	
	LCD_init();
	
    /* Replace with your application code */
    while (1) 
    {
    }
}

