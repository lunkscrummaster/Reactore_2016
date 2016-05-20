#ifndef InitialChargeSystem_h
#define InitialChargeSystem_h

#include "Arduino.h"
#include "Debug.h"

class InitialChargeSystem {
  public:
    InitialChargeSystem();
    void heartbeat();

    void enable(boolean en);  // disable when towing

    int  getCurrentPercent();    // gets current pressure as a percent
    void setTargetPercent(int p);

  private:
    byte state;
    void enterState(byte newState);

    boolean enabled;
    int  targetPressure;
};

#endif
