// Permission to copy is granted provided that this header remains intact. 
// This software is provided with no warranties.

// NOTE: This has been slightly modifed for Arduino

#ifndef LCD_H
#define LCD_H

#include <stdio.h>

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))
          
/*-------------------------------------------------------------------------*/

#define DATA_BUS PORTA		// port connected to pins 7-14 of LCD display
#define CONTROL_BUS_RS 2	// port connected to pins 4 and 6 of LCD disp.
#define CONTROL_BUS_E 3
#define RS 0				// pin number of uC connected to pin 4 of LCD disp.
#define E 0					// pin number of uC connected to pin 6 of LCD disp.

/*-------------------------------------------------------------------------*/

void delay_ms(int miliSec) { //for 8 Mhz crystal
	int i,j;
	for(i=0;i<miliSec;i++) {
		for(j=0;j<775;j++) {
			asm("nop");
		}
	}
}

/*-------------------------------------------------------------------------*/

void LCD_WriteCommand (unsigned char Command) {
	// CLR_BIT(CONTROL_BUS,RS);
	digitalWrite(CONTROL_BUS_RS, LOW);
	DATA_BUS = Command;
	// SET_BIT(CONTROL_BUS,E);
	digitalWrite(CONTROL_BUS_E, HIGH);
	asm("nop");
	// CLR_BIT(CONTROL_BUS,E);
	digitalWrite(CONTROL_BUS_E, LOW);

	delay_ms(2); // ClearScreen requires 1.52ms to execute
}

void LCD_ClearScreen(void) {
	LCD_WriteCommand(0x01);
}

void LCD_init(void) {
	delay_ms(100); //wait for 100 ms for LCD to power up

	// set R and S as output
	pinMode(CONTROL_BUS_RS, OUTPUT);
	pinMode(CONTROL_BUS_E, OUTPUT);

	DDRA = 0xFF; DATA_BUS = 0x00;

	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0f);
	LCD_WriteCommand(0x01);
	delay_ms(10);						 
}

void LCD_WriteData(char Data) {
	// SET_BIT(CONTROL_BUS,RS);
	digitalWrite(CONTROL_BUS_RS, HIGH);
	DATA_BUS = Data;
	// SET_BIT(CONTROL_BUS,E);
	digitalWrite(CONTROL_BUS_E, HIGH);
	asm("nop");
	//CLR_BIT(CONTROL_BUS,E);
	digitalWrite(CONTROL_BUS_E, LOW);
	delay_ms(1);
}

void LCD_Cursor(unsigned char column) {
	if ( column < 17 ) { // 16x2 LCD: column < 17; 16x1 LCD: column < 9
		LCD_WriteCommand(0x80 + column - 1);
		} else { // 6x2 LCD: column - 9; 16x1 LCD: column - 1
		LCD_WriteCommand(0xB8 + column - 9);
	}
}

void LCD_DisplayString( unsigned char column, const char* s) {
	LCD_ClearScreen();
	unsigned char c = column;

	while (*s)
	{
		LCD_Cursor(c++);
		Serial.print(*s);
		LCD_WriteData(*s++);	
	}
}

#endif // LCD_H
