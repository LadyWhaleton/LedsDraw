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

#define CONTROL_BUS_RS 2	// port connected to pins 4 and 6 of LCD disp.
#define CONTROL_BUS_E 3

#define BIT0 11
#define BIT1 10
#define BIT2 9
#define BIT3 8
#define BIT4 7
#define BIT5 6
#define BIT6 5
#define BIT7 4


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

// Added this function.
// This basically does what DATA_BUS = data did, except bitwise.  `
void writeToDataBus(unsigned char data)
{
  unsigned char currBit = 0;
  int pinNum = BIT0;

  for (int i = 0; i < 8; ++i)
  {
    currBit = GET_BIT(data, i);

    if (currBit > 0)
      digitalWrite(pinNum, HIGH);
    else
      digitalWrite(pinNum, LOW);

     pinNum--;
  }
}

void LCD_WriteCommand (unsigned char Command) {
	digitalWrite(CONTROL_BUS_RS, LOW);
  writeToDataBus(Command);
	digitalWrite(CONTROL_BUS_E, HIGH);
	asm("nop");
	digitalWrite(CONTROL_BUS_E, LOW);

	delay_ms(2); // ClearScreen requires 1.52ms to execute
}

void LCD_Cursor(unsigned char column) {
  if ( column < 17 ) { // 16x2 LCD: column < 17; 16x1 LCD: column < 9
    LCD_WriteCommand(0x80 + column - 1);
    } else { // 6x2 LCD: column - 9; 16x1 LCD: column - 1
    LCD_WriteCommand(0xB8 + column - 9);
  }
}

void LCD_WriteData(char Data) {
  digitalWrite(CONTROL_BUS_RS, HIGH);
  writeToDataBus(Data);
  digitalWrite(CONTROL_BUS_E, HIGH);
  asm("nop");
  digitalWrite(CONTROL_BUS_E, LOW);
  delay_ms(1);
}

void clearTopRow()
{
  LCD_Cursor(1);
  LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' ');
  LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' ');
  LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' ');
  LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' ');
}

void clearBottomRow()
{
  LCD_Cursor(17);
  LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' ');
  LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' ');
  LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' ');
  LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' '); LCD_WriteData(' ');
}

void LCD_ClearScreen(void) {
	//LCD_WriteCommand(0x01);
  clearTopRow();
  clearBottomRow();
  LCD_Cursor(1);
}

void LCD_init(void) {
  // set R and S as output
  pinMode(CONTROL_BUS_RS, OUTPUT);
  pinMode(CONTROL_BUS_E, OUTPUT);

  // DDRB = 0xFF; DATA_BUS = 0x00;
  pinMode(BIT0, OUTPUT);
  pinMode(BIT1, OUTPUT);
  pinMode(BIT2, OUTPUT);
  pinMode(BIT3, OUTPUT);
  pinMode(BIT4, OUTPUT);
  pinMode(BIT5, OUTPUT);
  pinMode(BIT6, OUTPUT);
  pinMode(BIT7, OUTPUT);
  
	delay_ms(100); //wait for 100 ms for LCD to power up

	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0f);
	LCD_WriteCommand(0x01);
	delay_ms(10);						 
}

void LCD_DisplayString( unsigned char column, const char* s) {
	//LCD_ClearScreen();
	unsigned char c = column;

	while (*s)
	{
		LCD_Cursor(c++);
		Serial.print(*s);
		LCD_WriteData(*s++);	
	}
}

#endif // LCD_H
