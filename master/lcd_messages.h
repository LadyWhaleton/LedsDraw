#ifndef LCD_MESSAGES_H
#define LCD_MESSAGES_H

#include "lcd.h"


void displayDefaultMenu()
{
	LCD_ClearScreen();
	LCD_DisplayString(1, "LEDsDraw");
	LCD_DisplayString(17, "1.Draw");
}

void displayMenuOption(int option)
{
	clearBottomRow();

	if (option == 1)
		LCD_DisplayString(17, "1.Draw");

	else if (option == 2)
		LCD_DisplayString(17, "2.Sync");

	else if (option == 3)
		LCD_DisplayString(17, "3.Reset");

	else if (option == 4)
		LCD_DisplayString(17, "4.Help");
}

void displayDrawMode()
{
	LCD_ClearScreen();
	LCD_DisplayString(1, "Drawing");
	LCD_DisplayString(17, "A.Save  B.Cancel");
}

void displaySyncMode()
{
	LCD_ClearScreen();
	LCD_DisplayString(1, "Syncing");
	LCD_DisplayString(17, "B.Unsync");
}


#endif