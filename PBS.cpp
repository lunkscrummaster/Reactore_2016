#include "PBS.h"

#include "AS.h"
#include "HAL.h"
#include "MAS.h"
#include "ORS.h"

#include "pinDefs.h"


#define SETTLING_COUNT   2   // number of heartbeat periods to wait


// PushbackSystem states
#define PBS_QUIET            0
#define PBS_READY1_SINKING   1
#define PBS_READY2_RAISING   2
#define PBS_READY3_SETTLING  3


PushbackSystem::PushbackSystem() {
}
/* PushbackSystem::heartbeat()
 *  This is called from the heartbeat function in the main program page
 *  
*/
void PushbackSystem::heartbeat() {
  Serial.println("Pushback System Heartbeat Started");
  Serial.print("Pushback initial state: "); Serial.println(pushback.state); 
  
#ifdef DEBUG
  if (debugFlagPBS) {
    DEBUG_PRINT_S(" PBS:");
    if (enabled)  DEBUG_PRINT_S("en-");
    else          DEBUG_PRINT_S("dis-");
    DEBUG_PRINT_I(digitalRead(oPushbackArmDownPin));
    DEBUG_PRINT_I(digitalRead(oPushbackArmUpPin));
    DEBUG_PRINT_S(" asl=");
    DEBUG_PRINT_I(digitalRead(oAirSpringLockout));
    DEBUG_PRINT_S(" dump=");
    DEBUG_PRINT_I(digitalRead(oPushbackDumpValve));

    DEBUG_PRINT_S(" in-");

    switch (state) {
      case PBS_QUIET:            DEBUG_PRINT_S("QUIET.");      break;
      case PBS_READY1_SINKING:   DEBUG_PRINT_S("SINKING:");   break;
      case PBS_READY2_RAISING:   DEBUG_PRINT_S("RAISING:");   break;
      case PBS_READY3_SETTLING:  DEBUG_PRINT_S("SETTLING:");  break;
    }
  }
#endif

  int son = halReadSonar_Pushback(2);//find value of pushback sonar
  
  Serial.print("Push Back Sonar Value: "); Serial.println(son); Serial.print("readySinkTo: "); Serial.println(readySinkTo);
  DEBUG_PRINT_I(son);
  DEBUG_PRINT_S("/");
  Serial.print("Pushback System State:  "); Serial.println(pushback.state);
  Serial.println("waiting for debug button");
  while(analogRead(aitestdebugtrigger) > 300);  Serial.println("continue"); // debug, waits untill high by user debug button
    
  switch (state) {
    /* Changes made May 20, 2016 by Trevor, Zach and Kevin
 *  readySinkTo is the lower limit. The machine will sink to this value, or below then start raising.
 *  the machine will raise until readyRaiseTo is less than or equal to the sonar value
 *  once this raise is complete, the machine enters a settling state for apprx 1sec, then is quiet
 */
    case PBS_READY1_SINKING:
#ifdef DEBUG
      if (debugFlagPBS)
        DEBUG_PRINT_I(readySinkTo);
#endif
/* Changes made May 20, 2016 by Trevor, Zach and Kevin
 *  readySinkTo is the lower limit. The machine will sink to this value, or below then start raising.
 *  the machine will raise until readyRaiseTo is less than or equal to the sonar value
 *  once this raise is complete, the machine enters a settling state for apprx 1sec, then is quiet
 */
      if (readySinkTo >= son) {
        // start raising
        digitalWrite(oAirSpringLockout, HIGH);  //written high to allow air into the springs
        enterState(PBS_READY2_RAISING);
      }
      break;

    case PBS_READY2_RAISING:
#ifdef DEBUG
      if (debugFlagPBS)
        DEBUG_PRINT_I(readyRaiseTo);
#endif
 // Serial.print("ready to raise= ");Serial.println(readyRaiseTo);
 // Serial.print("son = "); Serial.println(son);
      if (readyRaiseTo <= son) {
        // start settling
        enterState(PBS_READY3_SETTLING);
      }
      break;

    case PBS_READY3_SETTLING:
      if (settlingTimeout-- <= 0) {
        // finished settling
        enterState(PBS_QUIET);
        master.pushbackIsReady();
      }
      break;
  }
} //end pushback heartbeat2

/* PushbackSystem::enable
 *  This function is called from MasterSystem::heartbeat() 
 *  1. ????
 *  
*/
void PushbackSystem::enable(boolean en) {
  if (en) {
    if (!enabled) {
      // enabling
      enabled = true;
#ifdef DEBUG
      if (debugFlagPBS) DEBUG_PRINT_S(" PBS-enabled");
#endif
    } // end if
  } else {
    if (enabled) {
      // disabling
      enabled = false;
      enterState(PBS_QUIET);
#ifdef DEBUG
      if (debugFlagPBS) DEBUG_PRINT_S(" PBS-DISabled");
#endif
    } // end if (enabled)
  } // end else
} // end PushbackSystem::enable
/* PushbackSystem::enterState this takes a new state, or current state
 *  Function is called from MAS.cpp
 *  1. depending on the state, depends on what it does
 *  2. View indiviudual states for further comments
*/
void PushbackSystem::enterState(byte newState) {
  state = newState;
  switch (state) {
    case PBS_QUIET:
      halSetPushbackUpDown(0);
      outriggers.setBalanceMode(false);
      break;

    case PBS_READY1_SINKING:
      digitalWrite(oAirSpringLockout, LOW); // when low, suspension is availble to go down 
      accustat.pause();
      halSetPushbackUpDown(-1);
      break;

    case PBS_READY2_RAISING:
      halSetPushbackUpDown(1);
      outriggers.setBalanceMode(true); //beging to balance the machine
      break;

    case PBS_READY3_SETTLING:
      halSetPushbackUpDown(0);
      outriggers.setBalanceMode(false);
      settlingTimeout = SETTLING_COUNT;
      break;
  }
}//end enterState
/* goReady, is called from MAS.cpp 
 *  1. changes the accustate state to whatever state was passed in
 *  2. enters new state of PBS_READY1_SINKING which is ^^^
*/
void PushbackSystem::goReady(byte asMode, int sinkTo, int raiseTo) {
#ifdef DEBUG
  if (debugFlagPBS) DEBUG_PRINT_S(" PBS-GR");
#endif
  readySinkTo  = sinkTo;
  readyRaiseTo = raiseTo;
  accustat.setMode(asMode);
  enterState(PBS_READY1_SINKING);// this is above^^
}//end goReady



//******************** WHY IS THIS COMMENTED OUT?????????????
//void PushbackSystem::reReady() {
//  accustat.pause();
//  enterState(PBS_READY2_RAISING);
//}
