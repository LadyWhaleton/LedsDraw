#ifndef LCD_MESSAGES_H
#define LCD_MESSAGES_H

#include "lcd.h"


void displayDefaultMenu()
{
	LCD_ClearScreen();
	LCD_DisplayString(1, "LedsDraw");
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
		LCD_DisplayString(17, "4.Reset All");
}

void displayDrawMode()
{
	LCD_ClearScreen();
	LCD_DisplayString(1, "Drawing");
	LCD_DisplayString(17, "A.Save  D.Done");
}

void displayDrawModeAsk()
{
  LCD_ClearScreen();
  LCD_DisplayString(1, "Draw on which?");
  LCD_DisplayString(17, "1.F1  2.F2  3.F3");
}

void displaySyncMode()
{
	LCD_ClearScreen();
	LCD_DisplayString(1, "Syncing");
	LCD_DisplayString(17, "D.Unsync");
}

void displayFlag(char* f)
{
  LCD_DisplayString(16, f);
}

void displayClearFlag()
{
  LCD_DisplayString(16, " ");
}


#endif
