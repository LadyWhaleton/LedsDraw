// Permission to copy is granted provided that this header remains intact. 
// This software is provided with no warranties.

////////////////////////////////////////////////////////////////////////////////
// NOTE: This has been modified for Arduino Mega
// Returns '\0' if no key pressed, else returns char '1', '2', ... '9', 'A', ...
// If multiple keys pressed, returns leftmost-topmost one
// Keypad must be connected to port C
// Keypad arrangement
//        Px4 Px5 Px6 Px7
//	  col 1   2   3   4
//  row  ______________
//Px0 1	| 1 | 2 | 3 | A
//Px1 2	| 4 | 5 | 6 | B
//Px2 3	| 7 | 8 | 9 | C
//Px3 4	| * | 0 | # | D

#ifndef KEYPAD_H
#define KEYPAD_H

// Keypad Setup Values

#define ROW1 9 // Keypad Pin 5
#define ROW2 8 // Keypad Pin 6
#define ROW3 7 // Keypad Pin 7
#define ROW4 6 // Keypad Pin 8
#define COL1 13 // Keypad Pin 1
#define COL2 12 // Keypad Pin 2
#define COL3 11 // Keypad Pin 3
#define COL4 10 // Keypad Pin 4

void initKeyPad()
{
  pinMode(ROW1, INPUT);
  pinMode(ROW2, INPUT);
  pinMode(ROW3, INPUT);
  pinMode(ROW4, INPUT);

  pinMode(COL1, OUTPUT);
  pinMode(COL2, OUTPUT);
  pinMode(COL3, OUTPUT);
  pinMode(COL4, OUTPUT);
}

////////////////////////////////////////////////////////////////////////////////
//Functionality - Gets input from a keypad via time-multiplexing
//Parameter: None
//Returns: A keypad button press else '\0'
unsigned char GetKeypadKey() {

	// Check keys in col 1
	// Set Px4 to 0; others 1
  digitalWrite(COL1, LOW); digitalWrite(COL2, HIGH); 
  digitalWrite(COL3, HIGH); digitalWrite(COL4, HIGH);
 
	asm("nop"); // add a delay to allow PORTx to stabilize before checking
	if ( ~digitalRead(ROW1) ) { return '1'; }
	if ( ~digitalRead(ROW2) ) { return '4'; }
	if ( ~digitalRead(ROW3) ) { return '7'; }
	if ( ~digitalRead(ROW4) ) { return '*'; }

	// Check keys in col 2
	// Set Px5 to 0; others 1
  digitalWrite(COL1, HIGH); digitalWrite(COL2, LOW); 
  digitalWrite(COL3, HIGH); digitalWrite(COL4, HIGH);
 
	asm("nop"); // add a delay to allow PORTx to stabilize before checking
	if ( ~digitalRead(ROW1) ) { return '2'; }
	if ( ~digitalRead(ROW2) ) { return '5'; }
	if ( ~digitalRead(ROW3) ) { return '8'; }
	if ( ~digitalRead(ROW4) ) { return '0'; }

	// Check keys in col 3
	// Set Px6 to 0; others 1
  digitalWrite(COL1, HIGH); digitalWrite(COL2, HIGH); 
  digitalWrite(COL3, LOW); digitalWrite(COL4, HIGH);
 
	asm("nop"); // add a delay to allow PORTx to stabilize before checking
	if ( ~digitalRead(ROW1) ) { return '3'; }
	if ( ~digitalRead(ROW2) ) { return '6'; }
	if ( ~digitalRead(ROW3) ) { return '9'; }
	if ( ~digitalRead(ROW4) ) { return '#'; }

	// Check keys in col 4
	// Set Px7 to 0; others 1
  digitalWrite(COL1, HIGH); digitalWrite(COL2, HIGH); 
  digitalWrite(COL3, HIGH); digitalWrite(COL4, LOW);
 
	asm("nop"); // add a delay to allow PORTx to stabilize before checking
	if ( ~digitalRead(ROW1) ) { return 'A'; }
	if ( ~digitalRead(ROW2) ) { return 'B'; }
	if ( ~digitalRead(ROW3) ) { return 'C'; }
	if ( ~digitalRead(ROW4) ) { return 'D'; }
	
	return '\0';
}

#endif //KEYPAD_H
