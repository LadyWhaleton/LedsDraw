#include <LedControlMS.h>
#include "Agenda.h"
#include "keypad.h"
#include "helper.h"
#include <avr/io.h>

// ================ SHARED GLOBALS ACROSS TASKS ========================
char key;

// ================ SCHEDULER ========================
Agenda scheduler;
int task0, task1, task2, task3;

// ================ TILT SENSOR ========================
enum TiltDir {UP, DOWN, LEFT, RIGHT, CENTER} tiltDirection;

// ******************* TASKS*******************************************
// ********************************************************************

// ================ TASK LED MATRIX ========================
void Task_Keypad()
{
  key = GetKeypadKey();
}

// ================ TASK MAIN ===================================
enum T1_SM {MainInit, MainMenu, DrawModeAsk, DrawMode, SyncMode, Reset} mainState;
void Task_Main()
{
  static int menuOption;
  
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
        drawModeSetup(key);
        displayDrawMode();
        mainState = DrawMode;
      }
      break;

    case DrawMode:
      // blink the cursor
      blinkCursor();
      
      // if pattern has been shifted, redisplay the pattern
      if ( shiftPattern(EditedPattern, tiltDirection) )
        displayPattern(EditedPattern);
      
      // process key presses
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
        drawModeOn = false;
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
        plotPoint();

      // cursor movement
      else if (key == '2' || key == '8' || key == '4' || key == '6')
        moveCursor(key);
      
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
      if (key == '*' && !drawModeOn)
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
enum T3_SM {TiltDetect} tiltState;

// center: 0111, left: 0011, right: 1100, up: 0110, down: 1001
void Task_Tilt()
{
  /*
  Serial.print(digitalRead(TILT_B3), DEC);
  Serial.print(digitalRead(TILT_B2), DEC);
  Serial.print(digitalRead(TILT_B1), DEC);
  Serial.println(digitalRead(TILT_B0), DEC);
  */
  
  char b0 = digitalRead(TILT_B0);
  char b1 = digitalRead(TILT_B1);
  char b2 = digitalRead(TILT_B2);
  char b3 = digitalRead(TILT_B3);

  switch (tiltState)
  {
    case TiltDetect: 
      if (!b3 && !b2 && b1 && b0) // left
        tiltDirection = LEFT;
        
      else if (b3 && b2 && !b1 && !b0) // right
        tiltDirection = RIGHT;
        
      else if (!b3 && b2 && b1 && !b0) // up
        tiltDirection = UP;

      else if (b3 && !b2 && !b1 && b0) // down
        tiltDirection = DOWN;

      else 
        tiltDirection = CENTER;
      break;

    default:
      tiltState = TiltDetect;
  }
}

void setup() {
  // put your setup code here, to run once:
  LCD_init(); // LCD data lines on PORTL
  LedControl_init(); // init ledControl
  Pattern_init(); // init default patterns for LED matrix
  Keypad_init(); // Keypad on PortA
  
  Serial.begin(115200);

  task0 = scheduler.insert(Task_Keypad, TASK_KEYPAD_PERIOD, false);
  scheduler.activate(task0);
  
  mainState = MainInit;
  task1 = scheduler.insert(Task_Main, ONE_SEC/6, false);
  scheduler.activate(task1);

  ledState = Idle;
  task2 = scheduler.insert(Task_LedMat, TASK_LEDMAT_PERIOD, false);
  scheduler.activate(task2);

  tiltState = TiltDetect;
  task2 = scheduler.insert(Task_Tilt, ONE_SEC/8, false);
  scheduler.activate(task3);
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler.update();
}
