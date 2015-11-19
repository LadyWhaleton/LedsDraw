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

};

Pattern DefaultFrames[numFrames];
Pattern Frames[numFrames];

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


#endif
