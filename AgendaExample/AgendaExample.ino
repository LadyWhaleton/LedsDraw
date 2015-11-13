#include "Agenda.h"

Agenda scheduler;
int blink, blink2;

void blinker()
{
  Serial.println("Blink!");
}

void blinker2()
{
  Serial.println("Blink2!");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  blink = scheduler.insert(blinker, 1000000, false);
  scheduler.activate(blink);

  blink2 = scheduler.insert(blinker2, 1000000, false);
  scheduler.activate(blink2);
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler.update();

}
