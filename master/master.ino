#include "Agenda.h"
#include "lcd.h"
#include "keypad.h"
#include <avr/io.h>

// ================ GLOBALS ========================
const unsigned long ONE_SEC = 1000000;

Agenda scheduler;
int task1, task2;

void Task_DrawImage()
{
  //Serial.println("Draw Image!");
  /*
  LCD_Cursor(1);
  LCD_WriteData('H'); LCD_WriteData('e'); LCD_WriteData(44'l'); LCD_WriteData('l'); LCD_WriteData('o');
  */
  /*
  LCD_DisplayString(1, "12Hello!"); // some reason first two characters of any string gets truncated
  */

  char key = GetKeypadKey();

  if (key !=  '\0')
  {
    LCD_Cursor(18);
    LCD_WriteData(key);
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
  task1 = scheduler.insert(Task_DrawImage, ONE_SEC/8, false);
  scheduler.activate(task1);

  task2 = scheduler.insert(Task_Blink, ONE_SEC, false);
  scheduler.activate(task2);
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler.update();
}
