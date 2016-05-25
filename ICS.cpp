#include "ICS.h"

#include "HAL.h"
#include "pinDefs.h"

// ********************* what are these, and whats the difference between them?
#define ICS_QUIET     0 
#define ICS_RAISING   1
#define ICS_LOWERING  2


// converting percent to raw pressure (from analogRead)
#define PERCENT_TO_PRESSURE_OFFSET  160
#define PERCENT_TO_PRESSURE_FACTOR  2.8    // 2.8 corresponds to max 35 lbs

// This code deals with the pressure of the pushback spring tank aka the charge system, and not the resevoir
InitialChargeSystem::InitialChargeSystem() {
}

/* InitialCharge::heartbeat()
 *  This function is called from the main heartbeat() in the main program
 *  1. Reads the pressure in the precharge cylinder used for the pushback spring
 *  2. Put states to ICS_QUIET when done
*/
void InitialChargeSystem::heartbeat() {
//  Serial.println("Initial Charge System Heartbeat Started");
//  Serial.print("Initial Charge System State:  "); Serial.println(initcharge.state);
  
  int pres = analogRead(aiPrechargePin); //read pressure

//  Serial.print(" aiPrecharge Pressure:  "); Serial.println(pres);
  //while(pulseIn(ioTight_ball_sonar, HIGH) > 300);  Serial.println("continue"); // debug, waits untill pin 17 written high by user debug button

  DEBUG_PRINT_S(" ICS.");
  DEBUG_PRINT_I(digitalRead(oInitialChargeDown));
  DEBUG_PRINT_I(digitalRead(oInitialChargeUp));
  DEBUG_PRINT_S("=");
  DEBUG_PRINT_I(pres);
  DEBUG_PRINT_S(",pb=");
  DEBUG_PRINT_I(analogRead(aiAchievedPin));

  switch (state) {
    case ICS_RAISING:
      DEBUG_PRINT_S(",^:");
      DEBUG_PRINT_I(targetPressure);
      if (pres >= targetPressure)
        enterState(ICS_QUIET); //pressure is to high, enter quiet
      break;

    case ICS_LOWERING:
      DEBUG_PRINT_S(",v:");
      DEBUG_PRINT_I(targetPressure);
      if (pres <= targetPressure)
        enterState(ICS_QUIET); // if pressure it good, leave it
      break;

    case ICS_QUIET:
      DEBUG_PRINT_S(",q");
      break;
  } // end switch(state)
} // end InitialChargeSystem::heartbeat()

/* InitialChargeSystem::enable(boolean)
 *  This function is called from MasterSystem::UIModeChanged , UISystem::enterState 
 *  1. 
*/
void InitialChargeSystem::enable(boolean en) {
  if (en) {
    if (!enabled) {
      // enabling
      enterState(ICS_QUIET); //if called with true, and not enabled, enable it
      enabled = true;
    }
  } else { // if false passed in, and its enabled, disable it
    if (enabled) {
      // disabling
      enterState(ICS_QUIET);
      enabled = false;
    } 
  }
} // end InitialChargeSystem::enable

/* InitialChargeSystem::getCurrentPercent
 *  This function is not called
 *  What is the purpose and need for this????????????????????
*/
int InitialChargeSystem::getCurrentPercent() {
  int per = (int) ((analogRead(aiPrechargePin) - PERCENT_TO_PRESSURE_OFFSET) / PERCENT_TO_PRESSURE_FACTOR);
  if (per < 0)         per = 0;
  else if (per > 100)  per = 100;
  return per;
} // end InitialChargeSystem

/* InitialChargeSystem::setTargetPercent
 *  This function is called from MasterSystem::UIModeChanged , MasterSystem::UIVarChanged 
 *  // converting percent to raw pressure (from analogRead)
 * #define PERCENT_TO_PRESSURE_OFFSET  160
 * #define PERCENT_TO_PRESSURE_FACTOR  2.8    // 2.8 corresponds to max 35 lbs
 * 1. Sets the target pressure that is desired
 * 2. Increase pressure in pushback arm if needed
 * 3. Decrease pressure in pushback arm if needed
*/
void InitialChargeSystem::setTargetPercent(int per) {
  if (per < 0)         per = 0;
  else if (per > 100)  per = 100;

  targetPressure = (int) (PERCENT_TO_PRESSURE_OFFSET + per * PERCENT_TO_PRESSURE_FACTOR); //set pressure desired

  int pres = analogRead(aiPrechargePin);

  Serial.print(" setTargetPercent: ");  Serial.print(per);
  Serial.print(" targetPressure: ");  Serial.println(targetPressure);
  Serial.print(" aiPrechargePressure: ");  Serial.print(pres);
  
  if (targetPressure > pres)
    enterState(ICS_RAISING); //increase pressure in charge tank pushback arm if needed
  else if (targetPressure < pres)
    enterState(ICS_LOWERING); //lower pressure in charge tank pushback arm if needed
} // end InitialChargeSystem::setTargetPercent

/* InitialChargeSystem::enterState
 *  This function is called from InitialChargeSystem::heartbeat() , InitialChargeSystem::setTargetPercent , InitialChargeSystem::enable
 *  1.depending on the passed state, calls halSetInitialChargeSystem with -1,0,1 to move machine up/down or nothing
*/
void InitialChargeSystem::enterState(byte newState) {
  Serial.print("Init Charge System new state:  "); Serial.println(newState);
  switch (state = newState) {
    case ICS_QUIET:
      halSetInitialChargeUpDown(0);
      break;

    case ICS_RAISING:
      halSetInitialChargeUpDown(1);
      break;

    case ICS_LOWERING:
      halSetInitialChargeUpDown(-1);
      break;
  } // end switch (state)
} // end InitialChargeSystem::enterState


