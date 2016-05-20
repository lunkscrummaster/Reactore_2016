#ifndef PushbackSystem_H
#define PushbackSystem_H

#include "Arduino.h"
#include "Debug.h"


class PushbackSystem {
  public:
    PushbackSystem();
    void heartbeat(void);

    void enable(boolean en);

    // go to Ready position from Towing mode
    void goReady(byte asMode, int sinkTo, int raiseTo);
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
