#ifndef HAL_h
#define HAL_h

#include "Arduino.h"
#include "Debug.h"

void halSetup();

// reads Ball-In sonar and detects ball (TRUE if ball, FALSE if no ball)
boolean halIsBallIn(byte pin);

boolean halIsTowScrumSwitchInTowing();

void halSetInitialChargeUpDown(int dir);  // dir: +1 for up, -1 for down, 0 for neither
void halSetPushbackUpDown     (int dir);  // dir: +1 for up, -1 for down, 0 for neither

void halSetPushbackDumpValve(byte level);

// Sonar routines return raw microsecond durations, or zero for failure
// Divide microseconds by 58 to get cm, or divide by 147 to get inches
// Sonar is read up to 'tries' times to avoid returning zero
// (note that zero could be returned if all 'tries' attempts fail)

int halReadSonar_OutriggerLoose(byte tries);
int halReadSonar_OutriggerTight(byte tries);
int halReadSonar_Pushback      (byte tries);

#endif
