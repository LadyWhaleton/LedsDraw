#include "slave.h"

byte buf;
char c, findex, pindex;

// ===========================================================
// SCHEDULER
// ===========================================================
Agenda scheduler;
int task0, task1, task2;

// ===========================================================
// TASK MAIN: sets play animation flags
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
      else if (syncPlay && playFlippedV && !playFlippedH) animateFrames('1');
      else if (syncPlay && !playFlippedV && playFlippedH) animateFrames('2');
      else if (syncPlay && playFlippedV && playFlippedH) animateFrames('3');
      else if (!syncPlay) frameTime = FRAME_TIME;
      break;
      
    default:
      frameIndex = 0;
  }
}

// ===========================================================
// TASK SERIALEVENT: handles inputs / master commands
// ===========================================================
enum T2_SM {NoCommand, GotCommand, ProcessCommand} eventState;
void Task_SerialEvent()
{
  char inChar;
  byte rowPattern;
  
  // transitions
  switch (eventState)
  {
    case NoCommand:
      if (!awaitingOrder)
        eventState = GotCommand;
      break;

    case GotCommand:
      if (syncSetupDone)
        eventState = ProcessCommand;
      break;

    case ProcessCommand:
      if (awaitingOrder)
        eventState = NoCommand;
      break;
      
    default:
      eventState = NoCommand;
  }

  // actions
  switch (eventState)
  {
    case NoCommand:
      if (Serial.available())
      {
        inChar = (char) Serial.read();

        if (inChar == '2')
        {
          patternLoaded = false;
          pindex = 0; findex = 0; awaitingOrder = false;
        }
      }
      break;

    case GotCommand:
      while (Serial.available())
        syncSetup();
      break;

    case ProcessCommand:
      while (Serial.available())
        syncPoller();
      break;

    default:
      patternLoaded = false;
      pindex = 0; findex = 0; awaitingOrder = false;
  }
}

// ===========================================================
// SETUP, LOOP, SERIAL EVENT
// ===========================================================
void setup() 
{
  LedControl_init();
  resetFlags();

  pinMode(TEST_PIN, OUTPUT);
  pinMode(PLAY_BUTTON, INPUT);
  
  // put your setup code here, to run once:
  Serial.begin(115200);

  eventState = NoCommand;
  task2 = scheduler.insert(Task_SerialEvent, 1, false);
  scheduler.activate(task2);

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
  //SerialEvent();
  scheduler.update();
}

// problem was that I was sending the value as a char/string?
void syncSetup()
{ 
  buf = Serial.read();
  
  LoadedFrames[findex].row[pindex] = buf; pindex++;

  if (pindex == 8) { findex++; pindex = 0; }

  if (findex == 3) 
  { 
    flipFrames(false); flipFrames(true); invertFrames();
    patternLoaded = true; syncPlay = false;
    syncSetupDone = true;
    syncMode = true;
  }
}

void syncPoller()
{
  char inChar = (char) Serial.read();

  if (inChar == '*' && syncPlay) syncPlay = false;
  else if (inChar == '*' && !syncPlay) syncPlay = true;
  else if (inChar == 'D')
  { 
    digitalWrite(TEST_PIN, LOW);
    syncSetupDone = false;
    syncMode = false; syncPlay = false; awaitingOrder = true;
    c = '0'; 
  }
  // orientation stuff
  else if (inChar == 'V') { playReg = false; playFlippedV = true; playFlippedH = false; }
  else if (inChar == 'H') { playReg = false; playFlippedV = false; playFlippedH = true; }
  else if (inChar == 'I') { playReg = false; playFlippedV = true; playFlippedH = true; }
  else if (inChar == 'R') { playReg = true; playFlippedV = false; playFlippedH = false; digitalWrite(TEST_PIN, HIGH); }

}

