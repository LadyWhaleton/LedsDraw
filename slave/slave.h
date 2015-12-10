#ifndef SLAVE_H
#define SLAVE_H

#include <LedControlMS.h>
#include "Agenda.h"
#include "patterns.h"

// ===========================================================
// PIN MAPPING
// ===========================================================
#define LEDMAT0_DIN 11 // LEDMAT0 DIN, 51 (yellow)
#define LEDMAT0_CS 10 // LEDMAT0 CS, 53 (white)
#define LEDMAT0_CLK 9 // LEDMAT0 CLK, 52 (black)
#define LEDMAT_ADDR 0
#define TASK_LEDMAT_PERIOD 5000
#define PLAY_BUTTON 8
#define TEST_PIN 7

// ===========================================================
// GLOBALS
// ===========================================================
#define ONE_SEC 1000000
#define TASK_MAIN_PERIOD 5000
#define FRAME_TIME 200000

long frameTime = FRAME_TIME;
char frameIndex = 0;

// sync mode flags
bool playReg = true;
bool playFlippedV = false;
bool playFlippedH = false;
bool patternLoaded = false;
bool syncMode, syncPlay = false;
bool syncSetupDone = false;

// generic mode flags
bool playAnim = false;
bool awaitingOrder = true;

// ===========================================================
// LED MATRIX
// ===========================================================
// constructor parameters: dataPin, clkPin, csPin, numDevices)
LedControl lc = LedControl(LEDMAT0_DIN, LEDMAT0_CLK, LEDMAT0_CS, 1);

void LedControl_init()
{
  lc.shutdown(LEDMAT_ADDR, false);
  lc.setIntensity(LEDMAT_ADDR, 3);
  lc.clearDisplay(LEDMAT_ADDR);

  lc.setRow(LEDMAT_ADDR, 0, B00000000);
  lc.setRow(LEDMAT_ADDR, 1, B00000000);
  lc.setRow(LEDMAT_ADDR, 2, B00100100);
  lc.setRow(LEDMAT_ADDR, 3, B00100100);
  lc.setRow(LEDMAT_ADDR, 4, B00000000);
  lc.setRow(LEDMAT_ADDR, 5, B01000010);
  lc.setRow(LEDMAT_ADDR, 6, B00111100);
  lc.setRow(LEDMAT_ADDR, 7, B00000000);
}

void displayPattern(const Pattern& p)
{
  lc.clearDisplay(LEDMAT_ADDR);

  for (int i = 0; i < 8; i++)
    lc.setRow(LEDMAT_ADDR, i, p.row[i]);
}

// flag indicates whether to animate the flipped or reg versions
void animateFrames(char flag)
{
  // if frameTime > 0, stay on same frame. Decrement frameTime
  if (frameTime > 0) 
  {
    if (flag == '1') displayPattern(LoadedFramesV[frameIndex]);
    else if (flag == '2') displayPattern(LoadedFramesH[frameIndex]);
    else if (flag == '3') displayPattern(LoadedFramesI[frameIndex]);
    else displayPattern(LoadedFrames[frameIndex]);
    
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

void resetFlags()
{
  syncMode = false;
  syncPlay = false;
  playAnim = false;
  awaitingOrder = true;
  patternLoaded = false;
}

#endif
