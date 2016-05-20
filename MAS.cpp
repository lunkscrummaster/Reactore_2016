#include "MAS.h"

#include "AS.h"
#include "HAL.h"
#include "ICS.h"
#include "PBS.h"
#include "UIS.h"
#include "pinDefs.h"

#define MILLIS_MAX 4294967295

// values are raw sonar counts (microseconds)
#define INDIVIDUAL_SINK_RAISE      650, 675
#define POWER_SINK_RAISE           650, 800  // here are the pushback arm settings
#define STRENGTH_SINK_RAISE        405, 465


// limits for pushback arm during charge (raw values)
#define CHARGE_PRESSURE_TRIP  400   // if pressure over this, shutdown (400 means approx. 30 lbs)
#define CHARGE_DISTANCE_TRIP  635   // if sonar over this, shutdown


// how long to wait after Strength Charge completes (Success or Shutdown)
#define STRENGTH_POSTHIT_HEARTBEATS   (2 * HEARTBEATS_PER_SECOND)


// states of 'lastTowingSwitch'
#define MAS_LTS_UNKNOWN  0
#define MAS_LTS_OFF      1
#define MAS_LTS_ON       2


////////////////////////////// MasterSystem //////////////////////////////

MasterSystem::MasterSystem() {
  lastReadyState = UIS_SCRUM_INDIVIDUAL;
  strengthChargeTimeoutMillis = 0;
  strengthPosthitTimeoutHeartbeats = 0;
}
/* MasterSystem loop
 *  This function is called from the main loop
 *  1. Read PushBackSonar and Air pressure in main push back system
 *  2. If the sonar value is > CHARGE_DISTANCE_TRIP (635), ui.goStrengthPosthit - which just updates the screen display
 *  3. If the pressure in the tank is > CHARGE_PRESSURE_TRIP  (400) ,  which goes into ui.goStrengthPosthit which just updates the display
*/
void MasterSystem::loop() {
  Serial.println("Master loop started");
  Serial.print("lastUIstate: "); Serial.println(master.lastUIState);
  Serial.print("last ready state: "); Serial.println(master.lastReadyState);
  Serial.print("last tow state: "); Serial.println(master.lastTowSwitch);
  if (strengthChargeTimeoutMillis > 0) {
    // read sonar and pushback pressure
    int son  = halReadSonar_Pushback(2);
    int pres = analogRead(aiAchievedPin);
    
    Serial.print("Sonar PUSHBACK READING: "); Serial.println(son);
    Serial.print("Pressure in PUSHBACK: "); Serial.println(pres);
    Serial.println("waiting for debug button");
    while(analogRead(aitestdebugtrigger) > 300);  Serial.println("continue");
    
    
    DEBUG_PRINT_S("#");
    DEBUG_PRINT_I(millis());
    DEBUG_PRINT_S("\t");  DEBUG_PRINT_I(son);
    DEBUG_PRINT_S("\t");  DEBUG_PRINT_I(pres);
    DEBUG_PRINT_S("\t");  DEBUG_PRINT_I(
      strengthChargeTimeoutMillis >  32767L ?  32767 :
      strengthChargeTimeoutMillis < -32768L ? -32768 :
      strengthChargeTimeoutMillis);
    DEBUG_PRINT_S("\n");

    // check sonar distance
    Serial.print("Pushback Sonar Value: "); Serial.println(son);
    Serial.print("Pushback Air Pressure: "); Serial.println(pres);
    Serial.println("waiting for debug button");
    while(analogRead(aitestdebugtrigger) > 300);  Serial.println("continue");
    
    if (son < CHARGE_DISTANCE_TRIP) { //#define CHARGE_DISTANCE_TRIP  635   // if sonar over this, shutdown
      // if sonar too high,
      strengthChargeTimeoutMillis = 0;
      ui.goStrengthPosthit(UISPH_TOO_HIGH, son - CHARGE_DISTANCE_TRIP);//this just updates the lcd screen display
      return;
    }

    // check sonar speed
    //  long sonarMillis = millis();
    //  if (???) {
    //    // if sonar increasing too fast,
    //    strengthChargeTimeoutMillis = 0;
    //    ui.goStrengthPosthit(UISPH_TOO_FAST, 0);
    //    return;
    //  }
    //  lastSonarDistance = son;
    //  lastSonarMillis   = sonarMillis;

    // check pushback pressure
    if (pres > CHARGE_PRESSURE_TRIP) {  //#define CHARGE_PRESSURE_TRIP  400   // if pressure over this, shutdown (400 means approx. 30 lbs)
      // if pushback pressure too much,
      strengthChargeTimeoutMillis = 0;
      ui.goStrengthPosthit(UISPH_TOO_MUCH, pres - CHARGE_PRESSURE_TRIP);
      return;
    }
/* The code below does...????????????????????????????????????????
*/
    // check for Duration timeout
    unsigned long m = millis();
    unsigned long elapsedMillis = 0;
    if (m > lastMillis)
        elapsedMillis = m - lastMillis;
    else
        elapsedMillis = m - lastMillis + MILLIS_MAX;
    lastMillis = m;
    strengthChargeTimeoutMillis -= elapsedMillis;
    if (strengthChargeTimeoutMillis < 0) {
      strengthChargeTimeoutMillis = 0;
      digitalWrite(oSuccess, HIGH);
      ui.goStrengthPosthit(UISPH_SUCCESS, 0);//changes the screen to display success
      accustat.saveHiddenPeak();
    }
  } // end if (strengthChargeTimeoutMillis > 0)
}// end master loop

/* MasterSystem::heartbeat()
 *  This function is called from heartbeat() in the main page
 *  // states of 'lastTowingSwitch'
 * #define MAS_LTS_UNKNOWN  0
 * #define MAS_LTS_OFF      1
 * #define MAS_LTS_ON       2
 *  1. If the towing switch is in towing up, change LCD to display towing mode
 *  2. If not in towing up, change LCD to display last state
 *  3. Does some more stuff that I DON'T UNDERSTAND RIGHT NOW
*/
void MasterSystem::heartbeat() {
  
  Serial.println("Master System Heartbeat Started");   Serial.println("waiting for debug button");
  while(analogRead(aitestdebugtrigger) > 300);  Serial.println("continue"); // debug, waits untill high by user debug button

   
  // check towing switch
  if (halIsTowScrumSwitchInTowing()) {
    if (lastTowSwitch != MAS_LTS_ON) {
      // enter Towing mode
      lastTowSwitch = MAS_LTS_ON;
      ui.enterState(UIS_TOWING); //display towing state
    }
  } else {
    if (lastTowSwitch != MAS_LTS_OFF) {
      // exit Towing mode
      lastTowSwitch = MAS_LTS_OFF;
      ui.enterState(lastReadyState); //display last state
    }
  }

  boolean isLastUIStateNotTowing = lastUIState != UIS_TOWING;
  initcharge.enable              (isLastUIStateNotTowing);
  pushback.enable                (isLastUIStateNotTowing);
  accustat.enable                (isLastUIStateNotTowing);
  digitalWrite(oReservoirLockout, isLastUIStateNotTowing ? HIGH : LOW); //switches a boolean to high or low appropriately

  //***** the switch statement is lacking logic because it is switching a variable that has just been 
  // define to == 1 ?????????????????????????????

  // Things we do every heartbeat while UI is in each state:
  switch (lastUIState) {
    case UIS_TOWING:
      digitalWrite(oAirSpringLockout, LOW);// if towing, turn pin off
      ui.setVar(UIVN_TOWING_RESPRESS, analogRead(aiReservoirPin));
      break;

    case UIS_SCRUM_STRENGTH_POSTHIT:
      if (strengthPosthitTimeoutHeartbeats-- <= 1)
        ui.enterState(UIS_SCRUM_STRENGTH);
      break;

      // (other states do nothing)nnn
  } // end switch (lastUIState)
} // end MasterSystem::heartbeat()

/* MasterSystem::pushbackIsReady()
 *  This function is called from PushbackSystem::heartbeat()
 *  1. reset the timeout and accustat.resume() just does this : enterState(AS_PREHIT); in accustat
*/
// called by PushbackSystem when Ready cycle has finished
void MasterSystem::pushbackIsReady() {
  accustat.resume();
  strengthChargeTimeoutMillis = 0;
} // end MasterSystem::pushbackIsReady()

/* MasterSystem::accustatEnteringPosthit()
 *  This function is called from Accustat::enterState
 *  1. reset the timer
 *  2. writes the beeper low
 *  3. UI is the last ready state
*/
void MasterSystem::accustatEnteringPosthit() {
  strengthChargeTimeoutMillis = 0;
  digitalWrite(oSuccess, LOW);  // just to make sure
  if (lastReadyState != UIS_SCRUM_INDIVIDUAL)
    ui.enterState(lastReadyState);
} //end MasterSystem::accustatEnteringPosthit

/* MasterSystem::UIModeChanged
 *  This function is called from 
*/
void MasterSystem::UIModeChanged(byte uis) {
  lastUIState = uis;

  switch (lastUIState) {
    case UIS_SCRUM_INDIVIDUAL:
    case UIS_SCRUM_POWER:
    case UIS_SCRUM_STRENGTH:
      lastReadyState = lastUIState;
      digitalWrite(oSuccess, LOW);  // just to make sure
      break;
  }

  switch (lastUIState) {
    case UIS_SCRUM_INDIVIDUAL:   pushback.goReady(ASM_INDIVIDUAL, INDIVIDUAL_SINK_RAISE);  break;
    case UIS_SCRUM_POWER:        pushback.goReady(ASM_POWER,      POWER_SINK_RAISE);       break;

    case UIS_SCRUM_STRENGTH:
      initcharge.enable(true);
      initcharge.setTargetPercent(10 * ui.getVar(UIVM_STRENGTH_DIFFICULTY));
      pushback.goReady(ASM_STRENGTH, STRENGTH_SINK_RAISE);
      break;

    case UIS_SCRUM_STRENGTH_CHARGE:
      lastMillis = millis();
      strengthChargeTimeoutMillis = ui.getVar(UIVM_STRENGTH_DURATION) * 1000L;
      break;

    case UIS_SCRUM_STRENGTH_POSTHIT:
      strengthPosthitTimeoutHeartbeats = STRENGTH_POSTHIT_HEARTBEATS;
      break;
  }
}

void MasterSystem::UIVarChanged(byte uivn, int val) {
  DEBUG_PRINT_S(" MAS:UIV-");
  DEBUG_PRINT_I(uivn);
  DEBUG_PRINT_S("=");
  DEBUG_PRINT_I(val);
  DEBUG_PRINT_S(".");

  switch (uivn) {
    case UIVM_STRENGTH_DIFFICULTY:
      initcharge.setTargetPercent(10 * val);
      break;
  }
}
