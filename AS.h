#ifndef Accustat_H
#define Accustat_H

#include "Arduino.h"
#include "Debug.h"

#include "AVG.h"

// Accustat modes
#define ASM_INDIVIDUAL  0
#define ASM_POWER       1
#define ASM_STRENGTH    2


class Accustat {
  public:
    Accustat();
    void loop();
    void heartbeat(void);

    void enable(boolean en);  // turns off LED display when disabled

    void reset();  // resets sessionPeak, currentPeak

    // stop/start normal operation
    void pause();
    void resume();

    void setMode(byte m);    // one of ASM_*, determines Accustat sensitivity etc.

    void saveHiddenPeak();  // after Strength Charge completes, save the hidden peak as the Current Peak

    byte returnmode(); //this returns the mode at start up

  private:
    byte state;
    void enterState(byte newState);

    byte mode;  // one of ASM_*

    boolean hasSeenBall;  // TRUE if ball-in sensor has been triggered this cycle

    boolean isEnabled();

    Averager pbAvg;  // to smooth out pushback pressure readings

    int  sessionPeak;
    int  currentPeak;  // displayed on LED
    int  hiddenPeak;   // Power/Strength peak displayed at end of cycle, not during cycle
    int  precharge;
    int  lastReading;
    byte cooldownCounter;  // mainly for Individual mode (weak hits & bounces)

    byte  displayAlternateCountdown;  // counts heartbeats until next alternate
    byte  displayAlternateIndex;      // 0 = display session peak, >0 = display current peak

    void  displayHeartbeat();
};

#endif
