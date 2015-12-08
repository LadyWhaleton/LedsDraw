#include "master.h"
#include <avr/io.h>

// ================ SHARED GLOBALS ACROSS TASKS ========================
char key;

// ===========================================================
// SCHEDULER
// ===========================================================
Agenda scheduler;
int task0, task1, task2, task3;

// ================ TILT SENSOR ========================
enum TiltDir {UP, DOWN, LEFT, RIGHT, CENTER} tiltDirection;

// ===========================================================
// TASK MAIN
// ===========================================================
enum T1_SM {MainInit, MainMenu, DrawModeAsk, DrawMode, SyncMode, Reset} mainState;
void Task_Main()
{
  static int menuOption;

  char k = GetKeypadKey();
  
  switch (mainState)
  {
    case MainInit:
      menuOption = 1;
      displayDefaultMenu();
      mainState = MainMenu;
      break;
      
    case MainMenu:
      if (menuOption != MIN_OPTION && k =='4') // menu left
      {
        menuOption--;
        displayMenuOption(menuOption);
      }
      else if (menuOption != MAX_OPTION && k == '6') // menu right
      {
        menuOption++;
        displayMenuOption(menuOption);
      }
      
      else if (k == '5') // user selected something
      {
        if (menuOption == 1) // Draw mode
        {
          displayDrawModeAsk();
          mainState = DrawModeAsk;
        }
        else if (menuOption == 2) // Sync mode
        {
          Serial1.print("2&");

          // send the frames
          for (char i = 0; i < numFrames; ++i)
            for (char j = 0; j < 8; ++j)
              Serial1.write(Frames[i].row[j]);  
              
          displaySyncMode();
          mainState = SyncMode;
        }
      }

      if (k == '*' && !playAnim){ frameTime = FRAME_TIME; playAnim = true; }
      else if (k == '*' && playAnim) playAnim = false;
      
      break;

    case DrawModeAsk:
      if (k == 'A' || k == 'B' || k == 'C')
      {
        drawModeSetup(k);
        displayDrawMode();
        mainState = DrawMode;
      }

      else if (k == 'D')
      {
        displayDefaultMenu();
        mainState = MainMenu;
      }
      break;

    case DrawMode:
      // blink the cursor
      blinkCursor();
      
      // if pattern has been shifted, redisplay the pattern
      if ( shiftPattern(EditedPattern, tiltDirection) )
      {
        displayFlag("!");
        displayPattern(EditedPattern);
      }
      
      // process key presses
      if (k == 'A') // save
      {
        Frames[frameIndex] = EditedPattern;
        displayClearFlag();
      }
   
      else if (k == 'D') // done
      {
        // revert the pattern to it's original
        EditedPattern = Frames[frameIndex];
        displayPattern(EditedPattern);

        // return to and display main menu 
        displayDefaultMenu();
        drawModeOn = false;
        mainState = MainMenu;
      }

      else if (k == 'C') // clear
      {
        // clear the pattern, clear led matrix, redisplay cursor
        EditedPattern.clearPattern();
        displayPattern(EditedPattern);
        lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, true);

        displayFlag("!");
      }
      
      else if (k == '5') // draw or clear the point
        plotPoint();

      // cursor movement
      else if (k == '2' || k == '8' || k == '4' || k == '6')
        moveCursor(k);
      
      break;

    case SyncMode:
      if (k == 'D')
      {
        Serial1.print('D');
        playAnim = false;
        displayDefaultMenu();
        menuOption = 1;
        mainState = MainMenu;
      }

      else if (k == '*')
      {
        if (!playAnim){ frameTime = FRAME_TIME; playAnim = true; }
        else if (playAnim) playAnim = false;
        Serial1.print('*');
      }
      
      break;
      
    default:
      mainState = MainInit;
  }
}

// ===========================================================
// TASK LED MATRIX 
// ===========================================================
enum T2_SM {Wait, Drawing, Playing, Syncing} ledState;
void Task_LedMat()
{
  // transitions
  switch (ledState)
  {
    case Wait:
      if (playAnim && !drawModeOn)
        animateFrames();

      break;

    default:
      ledState = Wait;
      frameIndex = 0;
  }
}

// ===========================================================
// TASK TILT
// ===========================================================
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

// ===========================================================
// SETUP AND LOOP
// ===========================================================
void setup() {
  // put your setup code here, to run once:
  LCD_init(); // LCD data lines on PORTL
  LedControl_init(); // init ledControl
  Pattern_init(); // init default patterns for LED matrix
  Keypad_init(); // Keypad on PortA
  
  Serial.begin(115200);
  Serial1.begin(115200);
  
  mainState = MainInit;
  task1 = scheduler.insert(Task_Main, TASK_MAIN_PERIOD, false);
  scheduler.activate(task1);

  ledState = Wait;
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

