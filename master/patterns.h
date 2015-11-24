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
  bool emptyPattern()
  {
    for (int i = 0; i < 8; ++i)
      if (row[i] > 0)
        return false;
    return true;
  }

};

Pattern DefaultFrames[numFrames];
Pattern Frames[numFrames];
Pattern EditedPattern;

void Pattern_init()
{
  // pattern for the first frame;
  Pattern f1;
  f1.row[0] = B11111111;
  f1.row[1] = B10000001;
  f1.row[2] = B10000001;
  f1.row[3] = B10000001;
  f1.row[4] = B10000001;
  f1.row[5] = B10000001;
  f1.row[6] = B10000001;
  f1.row[7] = B11111111;

  // pattern for the second frame
  Pattern f2;
  f2.row[0] = B00000000;
  f2.row[1] = B01111110;
  f2.row[2] = B01000010;
  f2.row[3] = B01000010;
  f2.row[4] = B01000010;
  f2.row[5] = B01000010;
  f2.row[6] = B01111110;
  f2.row[7] = B00000000;

  // pattern for third frame
  Pattern f3;
  f3.row[0] = B00000000;
  f3.row[1] = B00000000;
  f3.row[2] = B00111100;
  f3.row[3] = B00100100;
  f3.row[4] = B00100100;
  f3.row[5] = B00111100;
  f3.row[6] = B00000000;
  f3.row[7] = B00000000;

  DefaultFrames[0] = f1;
  DefaultFrames[1] = f2;
  DefaultFrames[2] = f3;

  Frames[0] = f1;
  Frames[1] = f2;
  Frames[2] = f3;
}

void resetFrames()
{
  for (int i = 0; i < numFrames; ++i)
    Frames[i] = DefaultFrames[i];
}


// Determines the boundaries of the pattern
void getPatternBoundaries(const Pattern &p, int &top, int &bot, int &left, int &right)
{
  byte mask;
  byte rowPattern;
  int topMost = 10;
  int botMost = -1;

  int leftMost = 11;
  int rightMost = -2;
  
  int temp = 10;

  // find boundary for top and left
  for (int row = 0; row < 8; ++row)
  {
    Serial.print(row); Serial.print(" ");
    rowPattern = p.row[row];
    mask = B10000000;

    temp = 10;
    
    // check columns for this row
    for (int col = 0; col < 8; ++col)
    {
      Serial.print(col);
      // determine the farthest pixel on left for this row
      if (col < temp && mask & rowPattern > 0)
      {
          temp = col;
          break;
      }

       mask = mask >> 1;
    }
    Serial.println(" ");

    // check rows
    if (row < topMost && rowPattern > 0)
      topMost = row;

    // check if temp is the farthest pixel on left seen so far
    if (temp < leftMost)
      leftMost = temp;
  }

  
  // find boundary for bottom and right
  for (int row = 7; row >= 0; --row)
  {
    rowPattern = p.row[row];
    mask = B00000001;

    temp = -1;

    // check columns for this row
    for (int col = 7; col >= 0; --col)
    {
      // determine the farthest pixel on right for this row
      if (col > temp && mask & rowPattern > 0)
      {
          temp = col;
          break;
      }
       mask = mask >> 1;
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


#endif
