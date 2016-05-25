#ifndef OutriggerSystem_H
#define OutriggerSystem_H

#include "Arduino.h"
#include "Debug.h"
#include "Timer.h" 


class OutriggerSystem {
  public:
    OutriggerSystem();
    void loop(void);
    void heartbeat(void);
    void balanceISR(void);
    Timer balanceTimer;

#ifdef DEBUG_ORS
    void testLoop(void);
#endif

    void setBalanceMode(boolean en);
    boolean outriggersFirstPumpDone = false;

  private:
    void setupBalanceTimer(void);
    boolean inBalanceMode;
};

#endif
