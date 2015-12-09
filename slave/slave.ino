#include "slave.h"

byte buf;
char c, findex, pindex;

// ===========================================================
// SCHEDULER
// ===========================================================
Agenda scheduler;
int task0, task1;

// ===========================================================
// TASK MAIN: handles inputs / master commands
// ===========================================================
enum T0_SM {Init, Idle, Sync, Play} mainState;
void Task_Main()
{
  bool playButton = (bool) (~digitalRead(PLAY_BUTTON));
  
  switch (mainState)
  {
    case Init:
      mainState = Idle;
      break;
    case Idle:
      if (syncMode) { mainState = Sync; }
      else if (!syncMode && playButton && patternLoaded) { mainState = Play; playAnim = true; }
      break;

   case Play:
      if (playButton) { mainState = Idle; playAnim = false; }
      else if (syncMode) { mainState = Sync; playAnim = false; }
      break;
      
    case Sync:
      if (!syncMode)
          mainState = Idle;
      break;
      
    default:
      mainState = Init;
  }
}

// ===========================================================
// TASK LED MATRIX 
// ===========================================================
enum T1_SM {Wait, Drawing, Playing, Syncing} ledState;
void Task_LedMat()
{
  // transitions
  switch (ledState)
  {
    case Wait:
      if (syncMode) { ledState = Syncing; frameIndex = 0; frameTime = FRAME_TIME;}
      else if (!syncMode && patternLoaded && playAnim) { ledState = Playing; frameTime = FRAME_TIME; }
      break;

    case Playing:
      if (!playAnim && !syncMode) ledState = Wait;
      else if (syncMode) { ledState = Syncing; frameIndex = FRAME_TIME; }
      break;

    case Syncing:
      if (!syncMode) ledState = Wait;
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
      
    case Playing:
      animateFrames('0');
      break;
      
    case Syncing:
      if (syncPlay && playReg) animateFrames('0');
      else if (syncPlay && playFlippedV) animateFrames('1');
      else if (syncPlay && playFlippedH) animateFrames('2');
      else if (!syncPlay) frameTime = FRAME_TIME;
      break;
      
    default:
      frameIndex = 0;
  }
}

// ===========================================================
// SETUP, LOOP, SERIAL EVENT
// ===========================================================
void setup() 
{
  LedControl_init();
  resetFlags();

  pinMode(5, OUTPUT);
  pinMode(PLAY_BUTTON, INPUT);
  
  // put your setup code here, to run once:
  Serial.begin(115200);

  mainState = Idle;
  task0 = scheduler.insert(Task_Main, TASK_MAIN_PERIOD, false);
  scheduler.activate(task0);

  ledState = Wait;
  task1 = scheduler.insert(Task_LedMat, TASK_LEDMAT_PERIOD, false);
  scheduler.activate(task1);

}

void loop() 
{
  // put your main code here, to run repeatedly:
  SerialEvent();
  scheduler.update();
}

void SerialEvent() {
  
  while (Serial.available()) 
  {
    // we get another command here if we don't have an order
    // and if we were done with previous order
    if (awaitingOrder)
    {
      char inChar = (char)Serial.read();
      
      // if the incoming character is a '&', set a flag
      if (inChar == '2')
      {
        patternLoaded = false; orientationSet = false;
        pindex = 0; findex = 0; awaitingOrder = false;
        c = inChar;
      }
    }

    // already got a command
    else
    {
      if (c == '2') // sync mode
      {
        if (!patternLoaded) syncSetup();
        else 
        { 
          if (!orientationSet) getOrientation(); 
          else if (orientationSet) syncPoller();
        }
      }
      
    }
  } // end of while loop
}

// problem was that I was sending the value as a char/string?
void syncSetup()
{ 
  buf = Serial.read();
  
  LoadedFrames[findex].row[pindex] = buf; pindex++;

  if (pindex == 8) { findex++; pindex = 0; }

  if (findex == 3) 
  { 
    flipFrames(false); flipFrames(true); 
    patternLoaded = true; syncPlay = false;
    syncMode = true;
  }
}

void getOrientation()
{
  // digitalWrite(5, HIGH);
  char inChar = (char) Serial.read();

  // not receiving correct value for some reason
  if (inChar == 'V') { playReg = false; playFlippedV = true; playFlippedH = false; digitalWrite(5, LOW);}
  else if (inChar == 'H') { playReg = false; playFlippedV = false; playFlippedH = true; digitalWrite(5, LOW); }
  else { playReg = true; playFlippedV = false; playFlippedH = false; digitalWrite(5, HIGH); }

  orientationSet = true;
  
}

void syncPoller()
{
  // digitalWrite(5, HIGH);
  char inChar = (char) Serial.read();

  if (inChar == '*' && syncPlay) syncPlay = false;
  else if (inChar == '*' && !syncPlay) syncPlay = true;
  else if (inChar == 'D')
  { 
    syncMode = false; syncPlay = false; awaitingOrder = true; 
    c = '0'; 
  }

  orientationSet = false;
}

