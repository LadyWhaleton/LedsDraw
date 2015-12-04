#include "slave.h"

byte buf[8];
char c;
bool gotOrder = false; bool orderDone = false; 
bool doneLoading = false;

// ===========================================================
// SCHEDULER
// ===========================================================
Agenda scheduler;
int task0;

// ===========================================================
// TASK MAIN
// ===========================================================
enum T0_SM {Idle, Sync} mainState;
void Task_Main()
{
  switch (mainState)
  {
    case Idle:
      if (syncMode)
      {
        frameIndex = 0;
        mainState = Sync;
      }
 
      break;
    case Sync:
      if (doneLoading)
      {
        
      }
      break;
    default:
      mainState = Idle;
  }
}

// ===========================================================
// SETUP, LOOP, SERIAL EVENT
// ===========================================================
void setup() 
{
  LedControl_init();
   
  // put your setup code here, to run once:
  Serial.begin(115200);

  mainState = Idle;
  task0 = scheduler.insert(Task_Main, TASK_MAIN_PERIOD, false);
  scheduler.activate(task0);

  gotOrder = false; orderDone = true;
  doneLoading = false;
}

void loop() 
{
  // put your main code here, to run repeatedly:
  SerialEvent();
  setOrder();
  scheduler.update();
}

void SerialEvent() {
  while (Serial.available()) 
  {
    // we get another command here if we don't have an order
    // and if we were done with previous order
    if (!gotOrder && orderDone)
    {
      char inChar = (char)Serial.read();
      
      // if the incoming character is a period, set a flag
      // so the main loop can do something about it:
      if (inChar == '&')
        gotOrder = true;
      else 
        c = inChar;
    }
  }
}

// ===========================================================
// set the order
// ===========================================================
void setOrder()
{
  if (gotOrder && orderDone)
  {
    Serial.println(c);

    if (c == '1')
    {
      for (char i = 0; i < 3; ++i)
        loadPattern(i);
      syncMode == true;
      doneLoading = true;
      orderDone = false;
    }
  }
}
