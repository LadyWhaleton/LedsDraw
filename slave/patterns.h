// This is the slave edition of pattern.h

#ifndef PATTERNS_H
#define PATTERNS_H

#define numFrames 3

struct Pattern
{
  byte row[8];

  void clearPattern()
  {
    for (int i = 0; i < 8; ++i)
      row[i] = B00000000;
  }

  Pattern()
  {
    clearPattern();
  }

  Pattern& operator = (const Pattern& rhs)
  {
    if (this != &rhs)
    {
      for (int i = 0; i < 8; ++i)
        row[i] = rhs.row[i];
    }
  }

  // returns 1 if empty, 0 otherwise
  bool isEmpty()
  {
    for (int i = 0; i < 8; ++i)
      if (row[i] > 0)
        return false;
    return true;
  }

};

// ===========================================================
// Array of Patterns
// ===========================================================
Pattern LoadedPattern;
Pattern LoadedFrames[numFrames];
Pattern LoadedFramesV[numFrames]; // vertically flipped
Pattern LoadedFramesH[numFrames]; // horizontally flipped


// Determines the boundaries of the pattern
void getPatternBoundaries(const Pattern &p, char &top, char &bot, char &left, char &right)
{
  byte mask;
  byte rowPattern;
  char topMost = 10;
  char botMost = -1;

  char leftMost = 10;
  char rightMost = -2;
  
  char temp;

  // find boundary for top and left
  for (char row = 0; row < 8; ++row)
  {
    rowPattern = p.row[row];
    mask = B10000000;

    temp = 10;

    // check columns for this row
    for (char col = 0; col < 8; ++col)
    {
      // determine the farthest pixel on left for this row
      if ( col < temp && (mask & rowPattern) > 0)
          temp = col;
      
       mask = mask >> 1;
    }

    // check rows
    if (row < topMost && rowPattern > 0)
      topMost = row;

    // check if temp is the farthest pixel on left seen so far
    if (temp < leftMost)
      leftMost = temp;
  }
  
  // find boundary for bottom and right
  for (char row = 7; row >= 0; --row)
  {
    rowPattern = p.row[row];
    mask = B00000001;

    temp = -1;

    // check columns for this row
    for (char col = 7; col >= 0; --col)
    {
      // determine the farthest pixel on right for this row
      if (col > temp && (mask & rowPattern) > 0)
          temp = col;
          
       mask = mask << 1;
    }

    // check rows
    if (row > botMost && rowPattern > 0)
      botMost = row;

    // check if temp is the farthest pixel on right seen so far
    if (temp > rightMost)
      rightMost = temp;
  }

  top = topMost; bot = botMost; left = leftMost; right = rightMost;
}

// Return value indicates if the pattern was edited
// Takes in two parameters, the pattern to be modified and the shift direction
bool shiftPattern(Pattern& p, char shiftDir )
{
  bool patternChanged = false; 
  
  // first, check if the pattern is empty
  if ( p.isEmpty() )
    return patternChanged;

  // otherwise, get the boundaries of the pattern
  char top, bottom, left, right;
  getPatternBoundaries(p, top, bottom, left, right);

  if (shiftDir == 0 && top != 0) // shift up
  {
    patternChanged = true;
    for (char i = 1; i < 8; ++i)
      p.row[i-1] = p.row[i];
    p.row[7] = B00000000;
  }

  else if (shiftDir == 1 && bottom != 7) // shift down
  {
    patternChanged = true;
    for (char i = 7; i >= 1; --i)
      p.row[i] = p.row[i-1];
    p.row[0] = B00000000;
  }

  else if (shiftDir == 2 && left != 0) // shift left
  {
    patternChanged = true;
    for (char i = 0; i < 8; i++)
      p.row[i] = p.row[i] << 1;
  }

  else if (shiftDir == 3 && right != 7) // shift right
  {
    patternChanged = true;
    for (char i = 0; i < 8; i++)
      p.row[i] = p.row[i] >> 1;
  }

  return patternChanged;
}

byte reverseByte (byte b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

// Flips the pattern horizontally or vertically
// flag = 0 = horizontal, flag = 1 = vertical
void flipFrames(bool flag)
{
  if (flag) // vertical flip
  {
    for (char i = 0; i < numFrames; ++i)
    {
      LoadedFramesV[i].row[0] = LoadedFrames[i].row[7];
      LoadedFramesV[i].row[1] = LoadedFrames[i].row[6];
      LoadedFramesV[i].row[2] = LoadedFrames[i].row[5];
      LoadedFramesV[i].row[3] = LoadedFrames[i].row[4];
      LoadedFramesV[i].row[4] = LoadedFrames[i].row[3];
      LoadedFramesV[i].row[5] = LoadedFrames[i].row[2];
      LoadedFramesV[i].row[6] = LoadedFrames[i].row[1];
      LoadedFramesV[i].row[7] = LoadedFrames[i].row[0];
    }
  }

  else // horizontal flip
    for (char i = 0; i < numFrames; ++i)
      for (char j = 0; j < 8; ++j)
        LoadedFramesH[i].row[j] = reverseByte(LoadedFrames[i].row[j]);
}

#endif
