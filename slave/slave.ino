byte buf[8];
String s;
bool doneReading = false; 

void setup() {
  // put your setup code here, to run once:
   Serial.begin(115200);
   //Serial.println("starting.");
   doneReading = false;
}

void loop() {
  // put your main code here, to run repeatedly:
  SerialEvent();

  if (doneReading) // done
  {
    Serial.println(s);
    s = "";
    doneReading = false;
  }

}

void SerialEvent() {
  while (Serial.available()) {
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
