#ifndef MasterSystem_h
#define MasterSystem_h

#include "Arduino.h"
#include "Debug.h"

#include "AVG.h"

////////////////////////////// MasterSystem //////////////////////////////

#define AVE_ARRAY_SIZE  10

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

    void pushBackAve(void);
    void outriggerLooseAve(void);
    void outriggerTightAve(void);
    int getPushbackSonarAve(void);
    int getOutriggerLooseAve(void);
    int getOutriggerTightAve(void);
    
    long successStartTime = 0;
    
    boolean pushbackAveFull = false;
    boolean outriggerLooseAveFull = false;
    boolean outriggerTightAveFull = false;
    int pushbackArrayIndex = 0;
    int outriggerLooseIndex = 0;
    int outriggerTightIndex = 0;
    int pushbackSonar [AVE_ARRAY_SIZE] = {0};
    int pushbackSonarAve = 0;
    int outriggerLooseSonar [AVE_ARRAY_SIZE] = {0};
    int outriggerLooseSonarAve = 0;
    int outriggerTightSonar [AVE_ARRAY_SIZE] = {0};
    int outriggerTightSonarAve = 0;
    
  private:
    byte lastUIState;
    byte lastReadyState;
    byte lastTowSwitch;

    // used during loop() in STRENGTH CHARGE phase
    long lastMillis;
    long strengthChargeTimeoutMillis;
    byte strengthPosthitTimeoutHeartbeats;

};

#endif
