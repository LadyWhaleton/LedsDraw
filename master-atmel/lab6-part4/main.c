/*
 * lab6-part4.c
 *
 * Created: 10/19/2015 4:16:07 PM
 * Author : User1
 */ 

#include <avr/io.h>
#include "scheduler.h"
#include "keypad.h"
#include "lcd.h"
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
	
	PORTC = col;	// Pattern to display
	PORTD = ~(row);		// Row(s) displaying pattern
	
	return state;
}

int Task_GetInput(int state)
{
	char key = GetKeypadKey();
		
	if (key != '\0')
	{
		LCD_Cursor(1);
		LCD_WriteData(key);
	}
	
	return state;
}

// shift rows ~(0x01 << 1)

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00; // LCD control lines, output
	DDRC = 0xFF; PORTC = 0x00; // LCD_data lines, output
	
	DDRD = 0xF0; PORTD = 0x0F;; // initialize input ports for keypad
	
	LCD_init();

	tasksNum = 1;
	task taskList[tasksNum];
	tasks = taskList;

	
	// define tasks
	unsigned char i=0; // task number

	/*
	// Task that will displays image on the LED matrix
	tasks[i].state = -1;
	tasks[i].period = 150;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Task_DisplayImage;
	++i;
	*/
	
	// Task that will obtain input from the keypad
	tasks[i].state = -1;
	tasks[i].period = 100;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Task_GetInput;
	++i;


	TimerSet(50); // value set should be GCD of all tasks
	TimerOn();
	
    /* Replace with your application code */
    while (1) 
    {
    }
}

