#ifndef AVG_h
#define AVG_h

#include "Arduino.h"
#include "Debug.h"


#define AVG_NUM_READINGS  40


class Averager {
  public:
    Averager();

    void update(int r);

    void reset();

    int getAverage();

  private:
    byte  readingCount;
    byte  readingIndex;
    int   readings[AVG_NUM_READINGS];
    long  sum;
};

#endif
