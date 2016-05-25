#include "MAS.h"

#include "SS.h"
#include "AS.h"
#include "HAL.h"
#include "ICS.h"
#include "PBS.h"
#include "UIS.h"
#include "pinDefs.h"

#define MILLIS_MAX 4294967295

// values are raw sonar counts (microseconds)
/* **** Changed May 20, 2016. Trevor, Zach, and Lunk
 *  The  values below have been updated to the new sonar values for the new placement of the sonar.
 *  Minimum pushback distance is approx. 33cm, max is approx 40cm
*/
#define INDIVIDUAL_SINK_RAISE      345, 375   // 
#define POWER_SINK_RAISE           350, 380  // here are the pushback arm settings
#define STRENGTH_SINK_RAISE        329, 337


// limits for pushback arm during charge (raw values)
#define CHARGE_PRESSURE_TRIP  400   // if pressure over this, shutdown (400 means approx. 30 lbs)
#define CHARGE_DISTANCE_TRIP  390   // if sonar over this, shutdown


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
 *  2. If the sonar value is > CHARGE_DISTANCE_TRIP (390), ui.goStrengthPosthit - which just updates the screen display
 *  3. If the pressure in the tank is > CHARGE_PRESSURE_TRIP  (400) ,  which goes into ui.goStrengthPosthit which just updates the display
*/
void MasterSystem::loop() {
//  Serial.println("Master loop started");
//  Serial.print("lastUIstate: "); Serial.println(master.lastUIState);
//  Serial.print("last ready state: "); Serial.println(master.lastReadyState);
//  Serial.print("last tow state: "); Serial.println(master.lastTowSwitch);
  pushBackAve();
  outriggerLooseAve();
  outriggerTightAve();

  if (strengthChargeTimeoutMillis > 0) {
    // read sonar and pushback pressure
    //int son  = halReadSonar_Pushback(2);
    int son = getPushbackSonarAve();
    int pres = analogRead(aiAchievedPin);
    
    Serial.print("Sonar PUSHBACK READING: "); Serial.println(son);
    Serial.print("Pressure in PUSHBACK: "); Serial.println(pres);
//    Serial.println("waiting for debug button");
//    //while(pulseIn(ioTight_ball_sonar, HIGH) > 300);  Serial.println("continue");
    
    
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
//    Serial.print("Pushback Sonar Value: "); Serial.println(son);
//    Serial.print("Pushback Air Pressure: "); Serial.println(pres);
//    Serial.println("waiting for debug button");
//    //while(pulseIn(ioTight_ball_sonar, HIGH) > 300);  Serial.println("continue");
    
    if (son > CHARGE_DISTANCE_TRIP) { //#define CHARGE_DISTANCE_TRIP  390   // if sonar over this, shutdown
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
/* Code Added by Trevor and Zach
 *  The code below, keeps track of the timer for the strength charge push, which after they hold this for a certain 
 *  time, the sled is released, and it can be pushed. The sled push length is handled by anothter timer.
 *  Rollover is taken into account here.
*/
    // check for Duration timeout
     long m = millis();
     long elapsedMillis = 0;
    if (m > lastMillis)
        elapsedMillis = m - lastMillis;
    else
        elapsedMillis = m - lastMillis + MILLIS_MAX; //rollover
    lastMillis = m; 
    strengthChargeTimeoutMillis -= elapsedMillis;
    if (strengthChargeTimeoutMillis < 0) {
      strengthChargeTimeoutMillis = 0;
      digitalWrite(oSuccess, HIGH); //allow for the sled to move
      successStartTime = millis();
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
  
//  Serial.println("Master System Heartbeat Started");   Serial.println("waiting for debug button");
//  //while(pulseIn(ioTight_ball_sonar, HIGH) > 300);  Serial.println("continue"); // debug, waits untill high by user debug button

   
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

  /* **** CHAGNED MADE By Trevor Zach and Kevin
   *  Currently, the state lastUIState is always == 1, and the swtich statement below is a little weird
   * 
  */
  byte uiState = ui.getState(); // returns a value 0 -> 5 depending on the state
  byte ssState = sleep.getState(); //returns the value of the state 0 = Asleep, 1 = Awake

  if (uiState > 0 && ssState == 1) {
    accustat.enable(true);
    pushback.enable(true);
    if (uiState > 2) {
      initcharge.enable(true);
    }
  } else {
//    if (digitalRead(iTrailerPowerPin) == LOW) {  // TRUCK
//      accustat.enable(false);
//      pushback.enable(false);
//      initcharge.enable(false);
//    } // if the truck is hooked up, write above to low
  } // end if
/* Removed code below  
  boolean isLastUIStateNotTowing = lastUIState != UIS_TOWING;
  initcharge.enable              (isLastUIStateNotTowing);
  pushback.enable                (isLastUIStateNotTowing);
  accustat.enable                (isLastUIStateNotTowing);
*/

  
  digitalWrite(oReservoirLockout, ssState ? HIGH : LOW); //switches a boolean to high or low appropriately

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
  /* Changes made May 20, 2016 by trevor zach and lunk
   *  We are commenting all writing of oSuccess to low
   *  This will be done using a timer. So after there is a success, the timer will start.
   *  This timer will be edited by Kevin in the field to what his preference is.
  digitalWrite(oSuccess, LOW);  // just to make sure
  */
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
  /* Changes made May 20, 2016 by trevor zach and lunk
   *  We are commenting all writing of oSuccess to low
   *  This will be done using a timer. So after there is a success, the timer will start.
   *  This timer will be edited by Kevin in the field to what his preference is.
  digitalWrite(oSuccess, LOW);  // just to make sure
  */
      break;
  }
    Serial.println("uiModeChanged called goReady");
    Serial.print("lastUIState: "); Serial.println(lastUIState);
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

void MasterSystem::pushBackAve(){
  int average = 0;
  //int readingCount = 0;

  for(int i = 0; i < AVE_ARRAY_SIZE; i++){
     // if(pushbackSonar[i] > 0){
          average += pushbackSonar[i];
//          readingCount++;
//      }
  }
  pushbackSonarAve = average / AVE_ARRAY_SIZE;

//  Serial.print(" Pushback Ave: "); Serial.print(pushbackSonarAve); Serial.print(" Pushback Reading: "); Serial.println(pushbackSonar[(AVE_ARRAY_SIZE-1)]);
//  Serial.print("Pushback Sonar Direct: "); Serial.println(halReadSonar_Pushback(2));
}

    
void MasterSystem::outriggerLooseAve(){
  int average = 0;
  if(outriggerLooseAveFull == false){
    outriggerLooseSonar[outriggerLooseIndex] = pulseIn(ioOutriggerLooseSonar, HIGH);
    outriggerLooseIndex++;
    if(outriggerLooseIndex == AVE_ARRAY_SIZE)
        outriggerLooseAveFull = true;
    for(int i = 0; i < outriggerLooseIndex; i++){
        average += outriggerLooseSonar[i];
    }
    outriggerLooseSonarAve = average / outriggerLooseIndex;
  }else{
      for(int i = 0; i < AVE_ARRAY_SIZE; i++){
          if(i > 0)
              average += outriggerLooseSonar[i];
          if(i < AVE_ARRAY_SIZE-1)
              outriggerLooseSonar[i] = outriggerLooseSonar[i+1];
      }
      outriggerLooseSonar[AVE_ARRAY_SIZE-1] = pulseIn(ioOutriggerLooseSonar, HIGH);
      average += outriggerLooseSonar[AVE_ARRAY_SIZE-1];
      outriggerLooseSonarAve = average / AVE_ARRAY_SIZE;
  }
//  Serial.print(" Loose Ave: "); Serial.print(outriggerLooseSonarAve); Serial.print(" Loose Reading: "); Serial.println(outriggerLooseSonar[AVE_ARRAY_SIZE-1]);
}
    

void MasterSystem::outriggerTightAve(){
  int average = 0;
  if(outriggerTightAveFull == false){
        outriggerTightSonar[outriggerTightIndex] = pulseIn(ioOutriggerTightSonar, HIGH);
        outriggerTightIndex++;
        if(outriggerTightIndex == AVE_ARRAY_SIZE)
            outriggerTightAveFull = true;
        for(int i = 0; i < outriggerTightIndex; i++){
            average += outriggerTightSonar[i];
        }
        outriggerTightSonarAve = average / outriggerTightIndex;
  }else{  
      for(int i = 0; i < AVE_ARRAY_SIZE; i++){
          if(i > 0)
              average += outriggerTightSonar[i];
          if(i < AVE_ARRAY_SIZE-1)
              outriggerTightSonar[i] = outriggerTightSonar[i+1];
      }
      outriggerTightSonar[AVE_ARRAY_SIZE-1] = pulseIn(ioOutriggerTightSonar, HIGH);
      average += outriggerTightSonar[AVE_ARRAY_SIZE-1];
      outriggerTightSonarAve = average / AVE_ARRAY_SIZE;
  }
//  Serial.print(" Tight Ave: "); Serial.print(outriggerTightSonarAve); Serial.print(" Tight Reading: "); Serial.println(outriggerTightSonar[AVE_ARRAY_SIZE-1]);

}
    
int MasterSystem::getPushbackSonarAve(){
    pushBackAve();
    return pushbackSonarAve;  
}

int MasterSystem::getOutriggerLooseAve(){
    outriggerLooseAve();
    return outriggerLooseSonarAve;
}

int MasterSystem::getOutriggerTightAve(){
    outriggerTightAve();
    return outriggerTightSonarAve;
}



