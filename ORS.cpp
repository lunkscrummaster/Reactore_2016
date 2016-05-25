#include "ORS.h"

#include "HAL.h"
#include "PBS.h"
#include "MAS.h"
#include "pinDefs.h"

#define ORS_BALANCE_TRIP  5   // raises outrigger when sonars differ by more than this

#define BALANCE_PERIOD    100 //in microseconds

OutriggerSystem::OutriggerSystem() {
  //  Timer balanceTimer;
  ////setupBalanceTimer();
  //  balanceTimer.initialize(BALANCE_PERIOD);
  //  balanceTimer.attachInterrupt(balanceISR);
}
void OutriggerSystem::setupBalanceTimer() {
  //  balanceTimer.initialize(BALANCE_PERIOD);
  //  balanceTimer.attachInterrupt(balanceISR);
}
/*OutriggerSystem checks if the machine is inBalanceMode, and raises the loose up or down
   ORS_BALANCE_TRIP == 40 WHICH IS ABOVE ^^^
*/
void OutriggerSystem::loop() {
  //  Serial.println("Outrigger Loop started");
  if(outriggersFirstPumpDone == false){
        digitalWrite(oOutriggerLooseUp, HIGH);
        digitalWrite(oOutriggerTightUp, HIGH);
        delay(200);
        digitalWrite(oOutriggerLooseUp, LOW);
        digitalWrite(oOutriggerTightUp, LOW);
        outriggersFirstPumpDone = true;
  }
  
  //Serial.print("The balance mode is : "); Serial.println(outriggers.inBalanceMode);
  //  Serial.println("waiting for debug button");
  //  //while(pulseIn(ioTight_ball_sonar, HIGH) > 300);  Serial.println("continue"); // debug, waits untill high by user debug button
//&& pushback.getState() == PBS_QUIET
  if (inBalanceMode && pushback.getState() == PBS_QUIET) {
    int ld = master.getOutriggerLooseAve(); //was 2
    int td = master.getOutriggerTightAve();
//    Serial.print(" OutRiggers Loose: ");  Serial.print(ld); Serial.print(" OutRiggers Tight: "); Serial.print(td);
//    Serial.print(" Dif: "); Serial.println(ld - td);
    if (ld > 0 && td > 0) { // if both are reading good values
      if (digitalRead(oOutriggerLooseUp)) {
        if (ld - td > ORS_BALANCE_TRIP)
          digitalWrite(oOutriggerLooseUp, LOW);
      } else if (ld - td < - ORS_BALANCE_TRIP) {
        digitalWrite(oOutriggerLooseUp, HIGH);
        delay(100);
        digitalWrite(oOutriggerLooseUp, LOW);


      } else { // end 1st else
        setBalanceMode(false);
      }//ends last else

    } // end if (ld > 0 && td > 0)
    //    if(balancingDone == true){
    //      int dif = ld - td;      // differencing algorythm to appropriately set inBalanceMode
    //      if (dif < 0)
    //        dif = td - ld;
    //      if (dif < ORS_BALANCE_TRIP)
    //        setBalanceMode(false); Serial.println("setBalanceMode = FALSE");
    //      balancingDone = false;
    //    }

  } // end if(inBalanceMode)

}// end of outrigger system

#ifdef DEBUG_ORS
byte orsDownPin, orsUpPin;

void OutriggerSystem::testLoop(void) {
  if (! inBalanceMode) {
    digitalWrite(orsDownPin, ! digitalRead(iButtonPinDown));
    digitalWrite(orsUpPin,   ! digitalRead(iButtonPinUp));
  }
}
#endif

void OutriggerSystem::heartbeat(void) {
  DEBUG_PRINT_S(" oUD:");
#ifdef DEBUG_ORS
  if (debugORSSelect) {
    DEBUG_PRINT_S("tight");
    orsDownPin = oOutriggerTightDown;
    orsUpPin   = oOutriggerTightUp;
  } else {
    DEBUG_PRINT_S("loose");
    orsDownPin = oOutriggerLooseDown;
    orsUpPin   = oOutriggerLooseUp;
  }
#endif
  //  Serial.println("Outrigger System Heartbeat Started"); // ???? Comment these reading values when code is complete to save time
  int ol = master.getOutriggerLooseAve();
  int ot = master.getOutriggerTightAve();
  //  Serial.print("Loose Sonar Value "); Serial.println(ol);
  //  Serial.print("Tight Sonar Value "); Serial.println(ot);
  //  Serial.println("waiting for debug button");
  //  //while(pulseIn(ioTight_ball_sonar, HIGH) > 300);  Serial.println("continue"); // debug, waits untill high by user debug button

  DEBUG_PRINT_S(" ol=");
  DEBUG_PRINT_I(ol);
  DEBUG_PRINT_S("/");
  DEBUG_PRINT_I(digitalRead(oOutriggerLooseDown));
  DEBUG_PRINT_I(digitalRead(oOutriggerLooseUp));

  DEBUG_PRINT_S(" (");
  DEBUG_PRINT_I(ol - ot);
  DEBUG_PRINT_S(")");

  DEBUG_PRINT_S(" ot=");
  DEBUG_PRINT_I(ot);
  DEBUG_PRINT_S("/");
  DEBUG_PRINT_I(digitalRead(oOutriggerTightDown));
  DEBUG_PRINT_I(digitalRead(oOutriggerTightUp));

  if (inBalanceMode) {
    DEBUG_PRINT_S(" inbal:");
  }
}

/* setBalance Mode
    This function is called from the PushbackSystem::enterState
    1. This function takes a true/false, and sets the inBalance mode
    2. if it is inBalanceMode is true, and false has been passed, change inBalanceMode to false
    3. if inBalanceMode is false, and true has been passed, change inBalanceMode to true
*/
void OutriggerSystem::setBalanceMode(boolean en) {
  DEBUG_PRINT_S(" ORS.setBal=");
  DEBUG_PRINT_I(en);
  //  Serial.print("Outrigger balance mode: "); Serial.println(inBalanceMode);
  if (inBalanceMode) {
    if (! en) {
      // disabling balance mode, and end timer
      inBalanceMode = false;
      //      balanceTimer.stop();
      digitalWrite(oOutriggerLooseDown, LOW);
      digitalWrite(oOutriggerLooseUp,   LOW);
      digitalWrite(oOutriggerTightDown, LOW);
      digitalWrite(oOutriggerTightUp,   LOW);
    } // end if(! en)
  } else {
    if (en) {
      // enabling balance mode, and start timmer
      inBalanceMode = true;
      outriggersFirstPumpDone = false; 
      //      balanceTimer.restart();
    } // end if (en)
  } // end else
} // end setBalanceMode
