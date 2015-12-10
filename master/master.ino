#include "master.h"
#include <avr/io.h>

// ================ SHARED GLOBALS ACROSS TASKS ========================
char key;

// ===========================================================
// SCHEDULER
// ===========================================================
Agenda scheduler;
int task0, task1, task2, task3;

// ===========================================================
// TASK MAIN: handles inputs, sends commands to slave
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
          Serial1.print("2");

          // send the frames
          for (char i = 0; i < numFrames; ++i)
            for (char j = 0; j < 8; ++j)
              Serial1.write(Frames[i].row[j]);  

          displaySyncMode();
          syncModeOn = true;
          mainState = SyncMode;
        }

        else if (menuOption == 3) // reset currFrame to default
        {
          for (char i = 0; i < 8; ++i)
            Frames[frameIndex].row[i] = DefaultFrames[frameIndex].row[i];

          displayPattern(Frames[frameIndex]);
        }

        else if (menuOption == 4) // result all to default
        {
          for (char i = 0; i < numFrames; ++i)
            for (char j = 0; j < 8; ++j)
              Frames[i].row[j] = DefaultFrames[i].row[j];

          displayPattern(Frames[frameIndex]);
        }
      }
      

      // stop the animation upon entering DrawModeAsk or SyncMode
      if (k == '*' && !playAnim && mainState != DrawModeAsk){ frameTime = FRAME_TIME; playAnim = true; }
      else if (k == '*' && playAnim || mainState == DrawModeAsk) playAnim = false;
      else if (playAnim && mainState == SyncMode) playAnim = false;
      
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

      else if (k == 'B') // invert pattern
      {
        // invert pattern, display inverted pattern, redisplay cursor
        EditedPattern.invertPattern();
        displayPattern(EditedPattern);
        lc.setLed(LEDMAT_ADDR, cursorRow, cursorCol, true);

        displayFlag("!");
      }

      else if (k == 'C') // clear
      {
        // clear/fill the pattern, clear/fill led matrix, redisplay cursor
        if (EditedPattern.isEmpty()) EditedPattern.fillPattern();
        else EditedPattern.clearPattern();
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
      if (flipHorizEnable && !flipVertEnable) { Serial1.print('H'); }
      else if (!flipHorizEnable && flipVertEnable ) { Serial1.print('V'); }
      else if (flipHorizEnable && flipVertEnable) { Serial1.print('I'); }
      else Serial1.print('R');
    
      if (k == 'D')
      {
        Serial1.print('D');
        playAnim = false;
        syncModeOn = false;
        menuOption = 1;
        displayDefaultMenu();
        mainState = MainMenu;
      }

      else if (k == '*')
      {
        Serial1.print('*');
        if (!playAnim) playAnim = true; 
        else if (playAnim) playAnim = false;
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
      if (playAnim && !drawModeOn && !syncModeOn) { ledState = Playing; frameTime = FRAME_TIME; }
      else if (syncModeOn) { ledState = Syncing; frameTime = FRAME_TIME; frameIndex = 0; }
      else if (drawModeOn) ledState = Drawing;
      break;

    case Drawing:
      if (!drawModeOn) ledState = Wait;
      break;

    case Playing:
      if (!playAnim && !syncModeOn) ledState = Wait;
      else if (syncModeOn) { ledState = Syncing; frameTime = FRAME_TIME; frameIndex = 0; }
      break;

    case Syncing:
      if (!syncModeOn) { ledState = Wait; }
      break;
      
    default:
      ledState = Wait;
  }

  // actions
  switch (ledState)
  {
    case Wait:
      // do nothing
      break;

    case Drawing:
      blinkCursor();
      break;
      
    case Playing:
      animateFrames();
      break;

    case Syncing:
      if (playAnim) animateFrames();
      else if (!playAnim) frameTime = FRAME_TIME;
      break;
      
    default:
      frameIndex = 0;
  }
}

// ===========================================================
// TASK TILT
// ===========================================================
enum T3_SM {TiltWait, TiltDetect, TiltLeft, TiltRight, TiltUp, TiltDown} tiltState;

// center: 0100, left: 0011, right: 1100, up: 0110, down: 1001
void Task_Tilt()
{
  /*
  Serial.print(digitalRead(TILT_B3), DEC);
  Serial.print(digitalRead(TILT_B2), DEC);
  Serial.print(digitalRead(TILT_B1), DEC);
  Serial.println(digitalRead(TILT_B0), DEC);
  */
  
  bool b0 = (bool) digitalRead(TILT_B0);
  bool b1 = (bool) digitalRead(TILT_B1);
  bool b2 = (bool) digitalRead(TILT_B2);
  bool b3 = (bool) digitalRead(TILT_B3);

  switch (tiltState)
  {
    case TiltWait:
      if (drawModeOn) { tiltState = TiltDetect, tiltDirection = CENTER; }; 
      break;
      
    case TiltDetect: 
      if (!drawModeOn) { tiltState = TiltWait; return; }
        
      if (!b3 && !b2 && b1 && b0) { tiltState = TiltLeft; tiltDirection = LEFT; }
      else if (b3 && b2 && !b1 && !b0) { tiltState = TiltRight; tiltDirection = RIGHT; } 
      else if (!b3 && b2 && b1 && !b0) { tiltState = TiltDown; tiltDirection = DOWN; }
      else if (b3 && !b2 && !b1 && b0) { tiltState = TiltUp; tiltDirection = UP; }
      else tiltDirection = CENTER;

      // if pattern has been shifted, redisplay the pattern
      if ( drawModeOn && shiftPattern(EditedPattern, tiltDirection) )
      {
        displayFlag("!");
        displayPattern(EditedPattern);
      }
      break;
      
    case TiltLeft:
      if (!drawModeOn) { tiltState = TiltWait; return; }
      
      if (!b3 && !b2 && b1 && b0) { tiltDirection = CENTER; }
      else if (b3 && b2 && !b1 && !b0) { tiltState = TiltRight; tiltDirection = RIGHT; } 
      else if (!b3 && b2 && b1 && !b0) { tiltState = TiltDown; tiltDirection = DOWN; }
      else if (b3 && !b2 && !b1 && b0) { tiltState = TiltUp; tiltDirection = UP; }
      else { tiltState = TiltDetect; tiltDirection = CENTER; }

      if ( drawModeOn && shiftPattern(EditedPattern, tiltDirection) )
      {
        displayFlag("!");
        displayPattern(EditedPattern);
      }
      break;

    case TiltRight:
      if (!drawModeOn) { tiltState = TiltWait; return; }
      
      if (!b3 && !b2 && b1 && b0) { tiltState = TiltLeft; tiltDirection = LEFT; }
      else if (b3 && b2 && !b1 && !b0) { tiltDirection = CENTER; } 
      else if (!b3 && b2 && b1 && !b0) { tiltState = TiltDown; tiltDirection = DOWN; }
      else if (b3 && !b2 && !b1 && b0) { tiltState = TiltUp; tiltDirection = UP; }
      else { tiltState = TiltDetect; tiltDirection = CENTER; }

      if ( drawModeOn && shiftPattern(EditedPattern, tiltDirection) )
      {
        displayFlag("!");
        displayPattern(EditedPattern);
      }
      break;

    case TiltUp:
      if (!drawModeOn) { tiltState = TiltWait; return; }
      
      if (!b3 && !b2 && b1 && b0) { tiltState = TiltLeft; tiltDirection = LEFT; }
      else if (b3 && b2 && !b1 && !b0) { tiltState = TiltRight; tiltDirection = RIGHT; } 
      else if (!b3 && b2 && b1 && !b0) { tiltState = TiltDown; tiltDirection = DOWN; }
      else if (b3 && !b2 && !b1 && b0) { tiltDirection = CENTER; }
      else { tiltState = TiltDetect; tiltDirection = CENTER; }

      if ( drawModeOn && shiftPattern(EditedPattern, tiltDirection) )
      {
        displayFlag("!");
        displayPattern(EditedPattern);
      }
      break;
      
    case TiltDown:
      if (!drawModeOn) { tiltState = TiltWait; return; }
      
      if (!b3 && !b2 && b1 && b0) { tiltState = TiltLeft; tiltDirection = LEFT; }
      else if (b3 && b2 && !b1 && !b0) { tiltState = TiltRight; tiltDirection = RIGHT; } 
      else if (!b3 && b2 && b1 && !b0) { tiltDirection = CENTER; }
      else if (b3 && !b2 && !b1 && b0) { tiltState = TiltUp; tiltDirection = UP; }
      else { tiltState = TiltDetect; tiltDirection = CENTER; }

      if ( drawModeOn && shiftPattern(EditedPattern, tiltDirection) )
      {
        displayFlag("!");
        displayPattern(EditedPattern);
      }
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

  pinMode (1, OUTPUT);
  
  Serial.begin(115200);
  Serial1.begin(115200);
  
  mainState = MainInit;
  task1 = scheduler.insert(Task_Main, TASK_MAIN_PERIOD, false);
  scheduler.activate(task1);

  ledState = Wait;
  task2 = scheduler.insert(Task_LedMat, TASK_LEDMAT_PERIOD, false);
  scheduler.activate(task2);

  tiltState = TiltWait;
  task2 = scheduler.insert(Task_Tilt, ONE_SEC/8, false);
  scheduler.activate(task3);
}

void loop() {
  // put your main code here, to run repeatedly:
  detectLight();
  detectLight2();
  scheduler.update();
}

// use value 150
void detectLight()
{
  photocellReading = analogRead(photocellPin);  

  if (photocellReading <= 150)
    flipVertEnable = true;
  else
    flipVertEnable = false;

  /*
  Serial.print("Analog reading = ");
  Serial.println(photocellReading);     // the raw analog reading
  */
 
  // LED gets brighter the darker it is at the sensor
  // that means we have to -invert- the reading from 0-1023 back to 1023-0
  photocellReading = 1023 - photocellReading;
  //now we have to map 0-1023 to 0-255 since thats the range analogWrite uses
  brightness = map(photocellReading, 0, 1023, 0, 255);
  analogWrite(LSENSOR_PIN, brightness);
 
  // delay(100);
}

void detectLight2()
{
  //Serial.print("Analog reading2 = ");

  int brightness = analogRead(1);

  //Serial.println(brightness);

  if (brightness <= 350)
    flipHorizEnable = true;
  else
    flipHorizEnable = false;
  
  //Serial.println(brightness);
  analogWrite(1, brightness/2);
}

