#include "Agenda.h"
#include "lcd.h"
#include <avr/io.h>

// ================ GLOBALS ========================
const unsigned long ONE_SEC = 1000000;

Agenda scheduler;
int task1, task2;

void Task_DrawImage()
{
  Serial.println("Draw Image!");
  /*
  LCD_Cursor(1);
  LCD_WriteData('H'); LCD_WriteData('e'); LCD_WriteData('l'); LCD_WriteData('l'); LCD_WriteData('o');
  */

  LCD_DisplayString(2, "12Hello!"); // some reason first two characters of any string gets truncated
}

void Task_Blink()
{
  Serial.println("Blink!");
}

void setup() {
  // put your setup code here, to run once:

  /*
  DDRA = 0xFF; PORTA = 0x00; // LCD control lines, output
  DDRC = 0xFF; PORTC = 0x00; // LCD_data lines, output
    
  DDRD = 0xF0; PORTD = 0x0F;; // initialize input ports for keypad
   */

   LCD_init();
   
  
  Serial.begin(115200);
  task1 = scheduler.insert(Task_DrawImage, ONE_SEC/2, false);
  scheduler.activate(task1);

  task2 = scheduler.insert(Task_Blink, ONE_SEC, false);
  scheduler.activate(task2);
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler.update();
}
