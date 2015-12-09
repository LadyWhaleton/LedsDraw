#include "slave.h"

byte buf;
char c, findex, pindex;

// ===========================================================
// SCHEDULER
// ===========================================================
Agenda scheduler;
int task0, task1;

// ===========================================================
// TASK MAIN
// ===========================================================
enum T0_SM {Init, Idle, Sync} mainState;
void Task_Main()
{
  switch (mainState)
  {
    case Init:
      findex = 0; pindex = 0;
      mainState = Idle;
      break;
    case Idle:
      if (syncMode)
      {
        //frameIndex = 0;
        mainState = Sync;
      }
 
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
enum T1_SM {Wait, Drawing, Playing} ledState;
void Task_LedMat()
{
  
  // transitions
  switch (ledState)
  {
    case Wait:
      if (patternLoaded && syncPlay)
        animateFrames();

      else if (!syncPlay)
        frameTime = FRAME_TIME;
      
      break;
      
    default:
      frameIndex = 0;
      ledState = Wait;  
  }
}

// ===========================================================
// SETUP, LOOP, SERIAL EVENT
// ===========================================================
void setup() 
{
  LedControl_init();
  Pattern_init();
  resetFlags();

  pinMode(6, OUTPUT);
  
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
      if (inChar == '&')
      {
        patternLoaded = false;
        pindex = 0; findex = 0; awaitingOrder = false;
      }

      else 
        c = inChar;
    }

    // already got a command
    else
    {
      if (c == '2') // sync mode
      {
        if (!patternLoaded) syncSetup();
        else syncPoller();
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
  if (findex == 3) { patternLoaded = true; syncMode = true; syncPlay = false; }
          
}

void syncPoller()
{
  char inChar = (char) Serial.read();
  Serial.print(inChar);

  if (inChar == '*' && syncPlay) syncPlay = false;
  else if (inChar == '*' && !syncPlay) syncPlay = true;
  else if (inChar == 'D'){ syncMode = false; syncPlay = false; awaitingOrder = true; c = '0';}
}

