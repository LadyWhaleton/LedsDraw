#include "slave.h"

byte buf[8];
String s;
bool doneReading = false; 

// ===========================================================
// SCHEDULER
// ===========================================================
Agenda scheduler;
int task0;

// ===========================================================
// TASK MAIN
// ===========================================================
enum T1_SM {mainIdle} mainState;
void Task_Main()
{
  if (doneReading) // done
  {
    Serial.println(s);
    s = "";
    doneReading = false;
  }
  
  switch (mainState)
  {
    case mainIdle:
    
      break;
    default:
      mainState = mainIdle;
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

  mainState = mainIdle;
  task0 = scheduler.insert(Task_Main, TASK_MAIN_PERIOD, false);
  scheduler.activate(task0);

  doneReading = false;
}

void loop() 
{
  // put your main code here, to run repeatedly:
  SerialEvent();
}

void SerialEvent() {
  while (Serial.available()) 
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    //Serial.println("got something!");
    
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '1')
      doneReading = true;
    else 
      s += inChar;
  }
}
