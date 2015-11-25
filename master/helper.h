#define HELPER_H
#define HELPER_H

// moves the cursor
void moveCursor(char k)
{
	// Before we update the cursor, clear it's current position
	lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, false);

	// redisplay the Pattern
	displayPattern(EditedPattern);
        
	if (k == '2' && cursorRow != MROW1) cursorRow--; // move up
	else if (k == '8' && cursorRow != MROW8) cursorRow++; // move down
	else if (k== '4' && cursorCol != MCOL1) cursorCol--;// move left
	else if (k == '6' && cursorCol != MCOL8) cursorCol++;// move right  
        
	// Now, we can update the cursor position on the LED Matrix
	lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, true);
	ledCursorOn = true;
	cursorBlinkTime = CURSOR_TIME;
}

// this function is called when the user presses '5'
void plotPoint()
{
	// Check if there's already a dot there
	byte mask = B10000000;
	byte newRowPattern = EditedPattern.row[cursorRow];
	mask = newRowPattern & (mask >> cursorCol);

	if (mask > 0) // dot is drawn, so clear the dot
	    newRowPattern = newRowPattern & (~(B10000000 >> cursorCol));

	else // nothing drawn, so set the dot on
		newRowPattern = newRowPattern | (B10000000 >> cursorCol);

	// update the editedPattern
	EditedPattern.row[cursorRow] = newRowPattern;

	// display the new pattern
	displayPattern(EditedPattern);

	// redisplay the cursor
	lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, true);
	ledCursorOn = true;
	cursorBlinkTime = CURSOR_TIME;

	// indicate that picture has been changed.
	displayFlag("!");
}

#endif