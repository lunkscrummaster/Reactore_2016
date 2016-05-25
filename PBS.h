#ifndef PushbackSystem_H
#define PushbackSystem_H

#include "Arduino.h"
#include "Debug.h"


// PushbackSystem states
#define PBS_QUIET            0
#define PBS_READY1_SINKING   1
#define PBS_READY2_RAISING   2
#define PBS_READY3_SETTLING  3

class PushbackSystem {
  public:
    PushbackSystem();
    void heartbeat(void);

    void enable(boolean en);

    // go to Ready position from Towing mode
    void goReady(byte asMode, int sinkTo, int raiseTo);

    byte getState(void);
    //    void reReady();

#ifdef DEBUG
    byte state;
#endif

  private:

#ifndef DEBUG
    byte state;
#endif
    void enterState(byte newState);

    boolean enabled;
    int     readySinkTo, readyRaiseTo;
    byte    settlingTimeout;
};

#endif
