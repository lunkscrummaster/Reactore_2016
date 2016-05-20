#ifndef OutriggerSystem_H
#define OutriggerSystem_H

#include "Arduino.h"
#include "Debug.h"


class OutriggerSystem {
  public:
    OutriggerSystem();
    void loop(void);
    void heartbeat(void);

#ifdef DEBUG_ORS
    void testLoop(void);
#endif

    void setBalanceMode(boolean en);

  private:
    boolean inBalanceMode;
};

#endif
