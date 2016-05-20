#ifndef Debug_H
#define Debug_H

#include "Arduino.h"


#define HEARTBEATS_PER_SECOND  4


//#define DEBUG  // comment out to disable debugging code & messages


extern class MasterSystem         master;

extern class Accustat             accustat;
extern class CompressorSystem     comp;
extern class InitialChargeSystem  initcharge;
extern class InverterSystem       inverter;
extern class OutriggerSystem      outriggers;    // handles both outriggers
extern class PushbackSystem       pushback;
extern class SleepSystem          sleep;
extern class UISystem             ui;


// number of calls to heartbeat() per second
//#define HEARTBEATS_PER_SECOND  4


#ifdef DEBUG

//#define DEBUG_ORS   // Up/Down buttons control outrigger air

#ifdef DEBUG_ORS
extern boolean debugORSSelect;    // determines which outrigger is controlled by Up/Down buttons
#endif

extern boolean debugFlagAS,
       debugFlagCS,
       debugFlagICS,
       debugFlagPBS,
       debugFlagReady,
       debugFlagSleep,
       debugFlagSonar;

#define DEBUG_PRINT_S(s)  debugPrintS(s)     // print a string
#define DEBUG_PRINT_I(i)  debugPrintI(i)     // print an int

void debugPrintS(const char* s);
void debugPrintI(int i);

#else

#define DEBUG_PRINT_S(s)
#define DEBUG_PRINT_I(i)

#endif

#endif
