#ifndef HELPER_H
#define HELPER_H

#include <LedControlMS.h>
#include "Agenda.h"
#include "keypad.h"
#include "patterns.h"

// ===========================================================
// PIN MAPPING
// ===========================================================
#define MISO_PIN 50 
#define LEDMAT0_CLK 42 // LEDMAT0 CLK, 52 (black)
#define LEDMAT0_CS 43 // LEDMAT0 CS, 53 (white)
#define LEDMAT0_DIN 44 // LEDMAT0 DIN, 51 (yellow)
#define TILT_B0 46
#define TILT_B1 47
#define TILT_B2 48
#define TILT_B3 49

// ===========================================================
// LIGHTSENSOR
// ===========================================================
#define photocellPin 0
#define LSENSOR_PIN 0     
int brightness;
int photocellReading;     // the analog reading from the sensor divider

// ===========================================================
// GLOBALS
// ===========================================================
#define ONE_SEC 1000000
#define TASK_MAIN_PERIOD 150000
#define TASK_LEDMAT_PERIOD 50000
#define TASK_KEYPAD_PERIOD 50000
#define FRAME_TIME 200000
#define CURSOR_TIME 100000
#define MIN_OPTION 1
#define MAX_OPTION 4
#define LEDMAT_ADDR 0

bool playAnim = false;
bool drawModeOn = false;
bool syncModeOn = false;

enum rowNum {MROW1, MROW2, MROW3, MROW4, MROW5, MROW6, MROW7, MROW8};
enum colNum {MCOL1, MCOL2, MCOL3, MCOL4, MCOL5, MCOL6, MCOL7, MCOL8};

// for cursors
char cursorCol;
char cursorRow;

// ===========================================================
// LED MATRIX
// ===========================================================
// constructor parameters: dataPin, clkPin, csPin, numDevices)
LedControl lc = LedControl(LEDMAT0_DIN, LEDMAT0_CLK, LEDMAT0_CS, 1);

int frameIndex = 0;
long frameTime = FRAME_TIME;

bool ledCursorOn = false;
long cursorBlinkTime = CURSOR_TIME;

void LedControl_init()
{
  lc.shutdown(LEDMAT_ADDR, false);
  lc.setIntensity(LEDMAT_ADDR, 3);
  lc.clearDisplay(LEDMAT_ADDR);
}

void displayPattern(const Pattern& p)
{
  lc.clearDisplay(LEDMAT_ADDR);

  for (int i = 0; i < 8; i++)
    lc.setRow(LEDMAT_ADDR, i, p.row[i]);
}

void animateFrames()
{
  // if frameTime > 0, stay on same frame. Decrement frameTime
  if (frameTime > 0) 
  {
    displayPattern(Frames[frameIndex]);
    frameTime -= TASK_LEDMAT_PERIOD;
  }

  // otherwise, it's time to move onto the next frame
  else
  {
    frameTime = FRAME_TIME;
    
    if (frameIndex != numFrames-1)
      frameIndex++;

    else
      frameIndex = 0;
  }   
}

// ================ TASK MAIN STUFF ========================
// ===========================================================
// DRAW MODE ASK
// ===========================================================
void drawModeSetup(char k)
{
	if (k == 'A') frameIndex = 0;
	else if (k == 'B') frameIndex = 1;
	else if (k == 'C') frameIndex = 2;

	// display pattern first
	EditedPattern = Frames[frameIndex];
	displayPattern(EditedPattern);

	// display the cursor
	cursorCol = MCOL1; cursorRow = MROW1;
	lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, true);
	ledCursorOn = true;
	cursorBlinkTime = CURSOR_TIME*2;
        
	drawModeOn = true;
}

// ===========================================================
// DRAW MODE
// ===========================================================
void blinkCursor()
{
	if (ledCursorOn)
	{
		if (cursorBlinkTime <= 0)
		{
			cursorBlinkTime = CURSOR_TIME*2;
			ledCursorOn = false;
			lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, ledCursorOn);
        }

        else
          cursorBlinkTime -= TASK_LEDMAT_PERIOD;
	}

	else if (!ledCursorOn)
	{
		if (cursorBlinkTime < 0)
		{
          cursorBlinkTime = CURSOR_TIME;
          ledCursorOn = true;
          lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, ledCursorOn);
        }

		else
		cursorBlinkTime -= CURSOR_TIME;
	}
}

// moves the cursor
void moveCursor(char k)
{
	// Before we update the cursor, clear it's current position
	lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, false);

	// redisplay the Pattern
	displayPattern(EditedPattern);
        
	if (k == '2' && cursorRow != 0) cursorRow--; // move up
	else if (k == '8' && cursorRow != 7) cursorRow++; // move down
	else if (k== '4' && cursorCol != 0) cursorCol--;// move left
	else if (k == '6' && cursorCol != 7) cursorCol++;// move right  
        
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
