#include "Agenda.h"
#include "keypad.h"
#include "lcd_messages.h"
#include <avr/io.h>

// ================ GLOBALS ========================
#define ONE_SEC 1000000
#define MIN_OPTION 1
#define MAX_OPTION 4

Agenda scheduler;
int task1, task2;

enum T1_SM {MainInit, MainMenu, DrawMode, SyncMode, Reset} mainState;
void Task_Main()
{
  static int menuOption;
  char key = GetKeypadKey();

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
          displayDrawMode();
          mainState = DrawMode;
        }
        else if (menuOption == 2)
        {
          displaySyncMode();
          mainState = SyncMode;
        }
      }
      break;

    case DrawMode:
      if (key == 'B')
      {
        displayDefaultMenu();
        mainState = MainMenu;
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

void Task_Blink()
{
  //Serial.println("Blink!");
}

void setup() {
  // put your setup code here, to run once:

   LCD_init(); // LCD data lines on PORTL
   Keypad_init(); // Keypad on PortA
  
  Serial.begin(115200);
  task1 = scheduler.insert(Task_Main, ONE_SEC/8, false);
  scheduler.activate(task1);

  task2 = scheduler.insert(Task_Blink, ONE_SEC, false);
  scheduler.activate(task2);
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler.update();
}
