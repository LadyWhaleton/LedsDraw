#include <LedControlMS.h>
#include "Agenda.h"
#include "keypad.h"
#include "patterns.h"
#include "lcd_messages.h"
#include <avr/io.h>

// ================ GLOBALS ========================
#define ONE_SEC 1000000
#define TASK_LEDMAT_PERIOD 100000
#define FRAME_TIME 200000
#define MIN_OPTION 1
#define MAX_OPTION 4

// ================ PIN MAPPING ========================
#define MOSI_PIN 51 // LEDMAT DIN
#define MISO_PIN 50 
#define SCK_PIN 52 // LEDMAT CLK
#define SS_LEDMAT 53 // LEDMAT CS
#define LEDMAT_ADDR 0
#define TILT_B0 48
#define TILT_B1 49

// ================ SHARED GLOBALS ACROSS TASKS ========================
enum rowNum {MROW1, MROW2, MROW3, MROW4, MROW5, MROW6, MROW7, MROW8};
enum colNum {MCOL1, MCOL2, MCOL3, MCOL4, MCOL5, MCOL6, MCOL7, MCOL8};

char key;
int menuOption;

char cursorCol;
char cursorRow;

// ================ SCHEDULER ========================
Agenda scheduler;
int task1, task2, task3;

// ================ TILT SENSOR ========================
enum TiltDir {UP, DOWN, LEFT, RIGHT, CENTER} tiltDirection;

// ================ LED MATRIX ========================
// constructor parameters: dataPin, clkPin, csPin, numDevices)
LedControl lc = LedControl(MOSI_PIN, SCK_PIN, SS_LEDMAT, 1);

int frameIndex = 0;
long frameTime = FRAME_TIME;

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

// ******************* TASKS*******************************************
// ********************************************************************

// ================ TASK MAIN ========================
enum T1_SM {MainInit, MainMenu, DrawModeAsk, DrawMode, SyncMode, Reset} mainState;
void Task_Main()
{
  key = GetKeypadKey();

  switch (mainState)
  {
    case MainInit:
      menuOption = 1;
      displayDefaultMenu();
      mainState = MainMenu;
      break;
      
    case MainMenu:
      if (menuOption != MIN_OPTION && key =='4') // menu left
      {
        menuOption--;
        displayMenuOption(menuOption);
      }
      else if (menuOption != MAX_OPTION && key == '6') // menu right
      {
        menuOption++;
        displayMenuOption(menuOption);
      }
      
      else if (key == '5') // user selected something
      {
        if (menuOption == 1)
        {
          displayDrawModeAsk();
          mainState = DrawModeAsk;
        }
        else if (menuOption == 2)
        {
          displaySyncMode();
          mainState = SyncMode;
        }
      }
      break;

    case DrawModeAsk:
      if (key == 'A' || key == 'B' || key == 'C')
      {
        if (key == 'A') frameIndex = 0;
        else if (key == 'B') frameIndex = 1;
        else if (key == 'C') frameIndex = 2;

        // display pattern first
        EditedPattern = Frames[frameIndex];
        displayPattern(EditedPattern);

        // display the cursor
        cursorCol = MCOL1; cursorRow = MROW1;
        lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, true);
        
        displayDrawMode();
        mainState = DrawMode;
      }
      break;

    case DrawMode:
      if (key == 'A') // save
      {
        Frames[frameIndex] = EditedPattern;
        displayClearFlag();
      }
   
      else if (key == 'B') // done
      {
        // revert the pattern to it's original
        EditedPattern = Frames[frameIndex];
        displayPattern(EditedPattern);

        // return to and display main menu 
        displayDefaultMenu();
        mainState = MainMenu;
      }

      else if (key == 'C') // clear
      {
        // clear the pattern, clear led matrix, redisplay cursor
        EditedPattern.clearPattern();
        displayPattern(EditedPattern);
        lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, true);

        displayFlag("!");
      }
      
      else if (key == '5') // draw or clear the point
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

        // indicate that picture has been changed.
        displayFlag("!");
      }

      // cursor movement
      else if (key == '2' || key == '8' || key == '4' || key == '6')
      {
        // Before we update the cursor, clear it's current position
        lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, false);

        // redisplay the Pattern
        displayPattern(EditedPattern);
        
        if (key == '2' && cursorRow != MROW1) cursorRow--; // move up
        else if (key == '8' && cursorRow != MROW8) cursorRow++; // move down
        else if (key == '4' && cursorCol != MCOL1) cursorCol--;// move left
        else if (key == '6' && cursorCol != MCOL8) cursorCol++;// move right  
        
        // Now, we can update the cursor position on the LED Matrix
        lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, true);
      }
      
      break;

    case SyncMode:
      if (key == 'B')
      {
        displayDefaultMenu();
        mainState = MainMenu;
      }
      break;
      
    default:
      mainState = MainInit;
  }
}

// ================ TASK LED MATRIX ========================
enum T2_SM {Idle, Drawing, Playing, Syncing} ledState;
void Task_LedMat()
{
  
  // transitions
  switch (ledState)
  {
    case Idle:
      if (key == '*')
      {
        frameIndex = 0;
        frameTime = FRAME_TIME;
        ledState = Playing;
      }
      break;
    case Playing:
      if (key == '*')
        ledState = Idle;
      break;
    default:
      ledState = Idle;
  }

  // actions
  switch (ledState)
  {
    case Idle:
      // do nothing
      break;
    case Playing:
      animateFrames();
      break;
    default:
      ledState = Idle;
  }
}
// ================ TASK TILT ========================
enum T3_SM {InitPos, ShiftUP, ShiftDOWN, ShiftLEFT, ShiftRIGHT} tiltState;

// 00: left, 01: down, 10: up (default), 11: right
void Task_Tilt()
{
  /*
  Serial.print("Bit0: ");
  Serial.print(digitalRead(TILT_B0), DEC);
  Serial.print("; ");
  Serial.print("Bit1: ");
  Serial.println(digitalRead(TILT_B1), DEC);
  */

  char b0 = digitalRead(TILT_B0);
  char b1 = digitalRead(TILT_B1);

  switch (tiltState)
  {
    case InitPos: // 10 (default)
      if (!b1 && b0) // 01 (down)
      {
        tiltDirection = DOWN;
        tiltState = ShiftDOWN;
      }
      else if (!b1 && !b0) // 00 (left)
      {
        tiltDirection = LEFT;
        tiltState = ShiftLEFT;
      }
      else if (b1 && b0) // 11 (right)
      {
        tiltDirection = RIGHT;
        tiltState = ShiftRIGHT;
      }
      else
      {
        tiltDirection = CENTER;
        tiltState = InitPos;
      }
      break;
    case ShiftDOWN:

      break;
    default:
      tiltState = InitPos;
  }
}

void setup() {
  // put your setup code here, to run once:
  LCD_init(); // LCD data lines on PORTL
  LedControl_init(); // init ledControl
  Pattern_init(); // init default patterns for LED matrix
  Keypad_init(); // Keypad on PortA
  
  Serial.begin(115200);
  
  mainState = MainInit;
  task1 = scheduler.insert(Task_Main, ONE_SEC/8, false);
  scheduler.activate(task1);

  ledState = Idle;
  task2 = scheduler.insert(Task_LedMat, TASK_LEDMAT_PERIOD, false);
  scheduler.activate(task2);

  tiltState = InitPos;
  task2 = scheduler.insert(Task_Tilt, ONE_SEC/8, false);
  scheduler.activate(task3);
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler.update();
}
