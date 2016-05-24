#ifndef SleepSystem_h
#define SleepSystem_h

#include "Arduino.h"
#include "Debug.h"

#define SSS_ASLEEP  0 //System is ASLEEP
#define SSS_AWAKE   1 //System is AWAKE

class SleepSystem {
  public:
    SleepSystem();

    void heartbeat();

    void wakeup();

    byte getState();

  private:
    byte state;
    void enterState(byte newState);

    int timeoutHeartbeats;
};

#endif
