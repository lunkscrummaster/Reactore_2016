#ifndef MasterSystem_h
#define MasterSystem_h

#include "Arduino.h"
#include "Debug.h"

#include "AVG.h"

////////////////////////////// MasterSystem //////////////////////////////


class MasterSystem {
  public:
    MasterSystem();
    void heartbeat();
    void loop();      // used during Strength Charge phase

    void pushbackIsReady();          // called by Pushback when it's Ready
    void accustatEnteringPosthit();  // called by Accustat when hit is over

    // called by UISystem when its mode changes, or one of its vars change
    void UIModeChanged(byte uis);
    void UIVarChanged (byte uivn, int val);
    long successStartTime = 0;
  private:
    byte lastUIState;
    byte lastReadyState;
    byte lastTowSwitch;

    // used during loop() in STRENGTH CHARGE phase
    unsigned long lastMillis;
    long strengthChargeTimeoutMillis;
    byte strengthPosthitTimeoutHeartbeats;

};

#endif
