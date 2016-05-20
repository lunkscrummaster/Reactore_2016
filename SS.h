#ifndef SleepSystem_h
#define SleepSystem_h

#include "Arduino.h"
#include "Debug.h"

class SleepSystem {
  public:
    SleepSystem();

    void heartbeat();

    void wakeup();

  private:
    byte state;
    void enterState(byte newState);

    int timeoutHeartbeats;
};

#endif
