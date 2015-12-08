#include "slave.h"

char buf;
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
enum T1_SM {Stop, Drawing, Playing} ledState;
void Task_LedMat()
{
  
  // transitions
  switch (ledState)
  {
    case Stop:
      digitalWrite(6, HIGH);
      if (patternLoaded && syncPlay)
      {
        frameTime = FRAME_TIME;
        ledState = Playing;
      }
      break;
    case Playing:
      digitalWrite(6, LOW);

      if (!syncPlay)
        ledState = Stop;
      break;
    default:
      frameIndex = 0;
      ledState = Stop;
      
  }

  // actions
  switch (ledState)
  {
    case Stop:
      // do nothing
      break;
    case Playing:
      animateFrames();
      // lc.setRow(LEDMAT_ADDR, 3, B00001111);
      break;
    default:
      ledState = Stop;
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

  ledState = Stop;
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
        Serial.println(c);
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

void syncSetup()
{
  buf = (char) Serial.read();

  if (buf == 'D') { patternLoaded = true; syncMode = true; syncPlay = false; Serial.println("setup done"); }
  else if (buf == '&') { findex++; pindex = 0; }
  else { LoadedFrames[findex].row[pindex] = (byte) buf; pindex++; }
          
}

void syncPoller()
{
  char inChar = (char) Serial.read();
  Serial.print(inChar);

  if (inChar == '*' && syncPlay) syncPlay = false;
  else if (inChar == '*' && !syncPlay) syncPlay = true;
  else if (inChar == 'D'){ syncMode = false; syncPlay = false; awaitingOrder = true; c = '0';}
}

