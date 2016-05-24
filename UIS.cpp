#include "UIS.h"

#include "AS.h"
#include "CIS.h"
#include "HAL.h"
#include "ICS.h"
#include "MAS.h"
#include "SS.h"

#include "pinDefs.h"

#define MICROS_MAX 4294967295

// display chevrons to left and right of current variable being modified
#define CHEVRON_LEFT   -1
#define CHEVRON_RIGHT   4

#define BAD_VARNUM   0xff

////////////////////////////// MenuButton //////////////////////////////

#define MB_DEBOUNCE_MICROS  100000  // debounce time in microseconds

class MenuButton {
  public:
    MenuButton(byte pin);
    int update(unsigned long elapsedMicros);

  private:
    byte    inputPin;
    boolean last;
    long    level;
};

/* MenuButton
 *  This function is called from UISystem
 *  1. This function sets up the passed pin as in input pin as a pullup
*/
MenuButton::MenuButton(byte pin) {
  inputPin = pin;
  pinMode(inputPin, INPUT_PULLUP);
} // end MenuButton

/* Menu::update
 *  This function is called from UISystem::loop()
 *  1. This fucntion is a debounce routine. 
 *  2. Returns 1 for button press, -1 for button release, otherwise 0
*/
// Updates button state; returns +1 if button pressed,
// returns -1 if button released, otherwise returns zero.
int MenuButton::update(unsigned long elapsedMicros) {
  if (digitalRead(inputPin) == LOW) {
    // button is pressed
    level += elapsedMicros;
    if (level > MB_DEBOUNCE_MICROS) {
      // button has stayed pressed for a while
      level = MB_DEBOUNCE_MICROS;
      if (last == false) {
        // signal that button has been pressed
        last = true;
        return 1;
      }
    } // end if (level > MB_DEBOUNCE_MICROS)
  } else {
    // button not pressed
    level -= elapsedMicros;
    if (level < 0) {
      // button has stayed unpressed for a while
      level = 0;
      if (last == true) {
        // signal that button has been released
        last = false;
        return -1;
      }
    } // end iff (level < 0)
  } // end else

  return 0;  // no change to debounced state
} // end update

////////////////////////////// UISystem //////////////////////////////

UISystem::UISystem(LiquidCrystal &lc) :
  bMode(* new MenuButton(iButtonPinMode)),
  bUp  (* new MenuButton(iButtonPinUp  )),
  bDown(* new MenuButton(iButtonPinDown)),
  bSet (* new MenuButton(iButtonPinSet )),
  lcd  (lc)
{
  // user-modifiable var info:           col, row, width, val, min, max //////////
/* Changes made May 21 by Trevor and Zach  
 *  Previously, mins for UIVM_POWER_THRESHOLD,UIVM_STRENGTH_THRESHOLD, UIVM_STRENGTH_DIFFICULTY == 0
 *  Changed these to be == 1, just as a safety feature
*/
  setModVarInfo(UIVM_POWER_THRESHOLD,     15,  1, 2, 10, 3, 10);

  setModVarInfo(UIVM_STRENGTH_THRESHOLD,  15,  1, 2, 10, 3, 10);
  setModVarInfo(UIVM_STRENGTH_DIFFICULTY, 15,  2, 2,  2, 1, 10);
  setModVarInfo(UIVM_STRENGTH_DURATION,   15,  3, 2,  5, 1, 15);

  // non-modifiable var info: col, row, width //////////

  setNonModVarInfo(UIVN_TOWING_RESPRESS, 7, 2, 4);

  lcd.clear();
  lcd.noDisplay();
}

/* UISystem::setNonModVarInfo
 *  This function is called from UISystem(LiquidCrystal &lc)
 *  1. This sets the only stuff that the user can't change.
 *  2. this is only used in the towing mode for the resevoir pressure on display
*/
void UISystem::setNonModVarInfo(byte vn, byte col, byte row, byte width) {
  cols[vn] = col;
  rows[vn] = row;
  wide[vn] = width;
 // vals[vn] = 0; // this is the problem **** change made from 0 to the read
  vals[vn] = analogRead(aiReservoirPin);
} // end setNonModVarInfo

/* setModVarInfo
 *  This function is called from UISystem(LiquidCrystal &lc) 
 *  1. This function is is for the variables that the user can change
*/
void UISystem::setModVarInfo(byte vn, byte col, byte row, byte width, int val, int min, int max) {
  cols[vn] = col;
  rows[vn] = row;
  wide[vn] = width;
  vals[vn] = val;
  mins[vn] = min;
  maxs[vn] = max;
} // end setModVarInfo


/* UISystem::loop()
 *  This function is called from the main file loop
 *  1. First, finds the elapsedMicros since last called
 *  2. then it checks for set/mode button presses, and up down button presses
 *  3. Wakes up if there was a button press
 *  4. If set was pressed, reset the accustat values
 *  5. Then based on the current state, and the button presses, enter a new state
*/
void UISystem::loop() {
//  Serial.println("UI Loop started");
//  Serial.print("UI state: "); Serial.println(ui.state);
//  //while(pulseIn(ioTight_ball_sonar, HIGH) > 300);  Serial.println("continue"); // debug, waits untill pin 34 written high by user debug button

   long m = micros();
   long elapsedMicros = m - lastMicros;
   if (elapsedMicros < 0) //
   {
    elapsedMicros += MICROS_MAX;
   } else{
    lastMicros = m;
   } //don't rollover
  
  boolean modeWasPressed = bMode.update(elapsedMicros) > 0;
  boolean setWasPressed  = bSet .update(elapsedMicros) > 0;

  // if Up/Down buttons pressed, update 'adjust' value
  int adjust = 0;
  if (bUp  .update(elapsedMicros) > 0)  adjust++;
  if (bDown.update(elapsedMicros) > 0)  adjust--;

  //  if (modeWasPressed)  DEBUG_PRINT_S("UI: pressed Mode\n");
  //  if (setWasPressed )  DEBUG_PRINT_S("UI: pressed Set\n");
  //  if (adjust != 0)     DEBUG_PRINT_S("UI: pressed Up/Down\n");
  if (modeWasPressed) Serial.println("MODE BUTTON WAS PRESSED");
  if (setWasPressed) Serial.println("SET WAS PRESSED");
  if (adjust != 0)   Serial.println("UP OR DOWN WAS PRESSED");
  // wakeup sleep system if a button was pressed

/* **** Possible changes made to the LCD below. May 20, 2016 by Trevor Zach and Kevin1
 *  The screen sometimes makes a fuzzy display due to possible EMI
 *  1. By enterState(state); we will reset the screen every time a button is pressed.
 *  2. This is just a possible implementation.
*/
/* **** CHANGED By Trevor Zach and Kevin
 *  We want the machine to be SSS_AWAKE while the truck is plugged in.
 *  This can be done by looking at the truck power connection as another way to wake it up.
 *  Original if (modeWasPressed || setWasPressed || adjust != 0){
*/
  if (modeWasPressed || setWasPressed || adjust != 0 || digitalRead(iTrailerPowerPin) == HIGH){
    // TRUCK original: digitalRead(iTrailerPowerPin) == LOW
    sleep.wakeup();
    Serial.println(" button press check is calling wakeup ");
    Serial.print(" modeWasPressed: "); Serial.print(modeWasPressed);
    Serial.print(" setWasPressed: "); Serial.print(setWasPressed);
    Serial.print("  adjust: "); Serial.print(adjust);
    Serial.print("  iTrailerPowerPin: "); Serial.println(digitalRead(iTrailerPowerPin));
  // ****  enterState(state); //reset the screen
  } // end if button was pressed

    // ???? investigate the sonar pushback safety stystem of when it should shut off because 
    // it is going to fast

  // reset the Accustat when the Set button is pressed
  if (setWasPressed)
    accustat.reset();

#ifdef DEBUG_ORS
  if (setWasPressed)
    debugORSSelect = ! debugORSSelect;
#endif
  /*
     LIST OF POSSIBLE STATES BELLOW
     // UISystem states
    #define UIS_TOWING                   0      #define UIS_SCRUM_POWER              1
    #define UIS_SCRUM_INDIVIDUAL         2      #define UIS_SCRUM_STRENGTH           3
    #define UIS_SCRUM_STRENGTH_CHARGE    4      #define UIS_SCRUM_STRENGTH_POSTHIT   5
  */
  //Serial.print("UI System State:  "); Serial.println(state);
  switch (state) {
    case UIS_SCRUM_POWER:
      if (modeWasPressed) {
        enterState(UIS_SCRUM_STRENGTH);
        accustat.reset();
      } else if (setWasPressed) {
        switch (cur_var) {
          case UIVM_POWER_THRESHOLD:  changeVar(BAD_VARNUM);            break;
          default:                    changeVar(UIVM_POWER_THRESHOLD);  break;
        }
      }
      break;

    case UIS_SCRUM_INDIVIDUAL:
      if (modeWasPressed) {
        enterState(UIS_SCRUM_POWER);
        accustat.reset();
      } // Set, Adjust ignored
      break;

    default:  // a Strength mode
      if (modeWasPressed) {
        enterState(UIS_SCRUM_INDIVIDUAL);
        accustat.reset();
      } else if (state == UIS_SCRUM_STRENGTH && setWasPressed) {
        switch (cur_var) {
          case UIVM_STRENGTH_THRESHOLD:   changeVar(UIVM_STRENGTH_DIFFICULTY);  break;
          case UIVM_STRENGTH_DIFFICULTY:  changeVar(UIVM_STRENGTH_DURATION);    break;
          case UIVM_STRENGTH_DURATION:    changeVar(BAD_VARNUM);                break;
          default:                        changeVar(UIVM_STRENGTH_THRESHOLD);   break;
        } // end switch (cur_var)
      }
      break;
  } // end switch (state)

  // adjust current variable (if one is selected)
  if (adjust != 0 && cur_var != BAD_VARNUM)
    setVar(cur_var, getVar(cur_var) + adjust);
} // end loop

/*  UISystem::heartbeat()
     Called from the main page heartbeat function
     // output-only (non-user-modifiable) display vars
  #define UIVN_TOWING_RESPRESS        4  // display reservoir pressure while towing
  #define UIVN__NUM                   5,
  // UISystem states
  #define UIS_TOWING                   0
  #define UIS_SCRUM_POWER              1
  #define UIS_SCRUM_INDIVIDUAL         2
  #define UIS_SCRUM_STRENGTH           3
  #define UIS_SCRUM_STRENGTH_CHARGE    4
  #define UIS_SCRUM_STRENGTH_POSTHIT   5
   1. Depending on the state, enter showvar(state) which is below
   The variables that are shown on the screen are the threshold, difficulty, duration etc.
*/
void UISystem::heartbeat() {
  // show visible variable(s)
  //Serial.print("Begin UISystem heartbeat state: "); Serial.println(ui.state);
  switch (state) {
    case UIS_TOWING:
      showVar(UIVN_TOWING_RESPRESS);
      break;

    case UIS_SCRUM_POWER:
      showVar(UIVM_POWER_THRESHOLD);
      break;

    case UIS_SCRUM_INDIVIDUAL:
      // (no vars to show)
      break;

    case UIS_SCRUM_STRENGTH:
      showVar(UIVM_STRENGTH_THRESHOLD);
      showVar(UIVM_STRENGTH_DIFFICULTY);
      showVar(UIVM_STRENGTH_DURATION);
      break;
  }
}// end UISystem::heartbeat()

/* setVar
 *  This function is called from MasterSystem::heartbeat() , UISystem::loop()
 *  1. This takes the value that was passed, stores it in the arrary
 *  2. then returns true of false depending on if the variable changed
*/
boolean UISystem::setVar(byte vn, int val) {
  if (val < mins[vn])  mins[vn] = val; //changed May 23; from val = min[vn]
  if (val > maxs[vn])  maxs[vn] = val;

  boolean hasChanged = vals[vn] != val;
  vals[vn] = val;

  if (hasChanged)
    master.UIVarChanged(vn, val);

  return hasChanged;
} // end setVar

/* getVar
 *  This fucntion is called from Accustat::heartbeat() , MasterSystem::UIModeChanged( , UISystem::loop()
 *  1.Returns that value depending on what state was passed in
*/
int UISystem::getVar(byte vn) {
  return vals[vn];
} //getVar

/*  UISystem::showVar(byte)
     This is called from UISystem::heartbeat() which is above^^
     1. Depending on the state, it displays the
*/
void UISystem::showVar(byte vn) {
  lcd.setCursor(cols[vn], rows[vn]);

  int val = vals[vn];

  // print leading spaces
  int w = wide[vn] - (val > 9) - (val > 99) - (val > 999);
  while (--w > 0)
    lcd.print(' ');//print spaces before the name

  lcd.print(val);//prints the value

  // print suffix
  switch (vn) {
    case UIVM_POWER_THRESHOLD:      // (fall into next)
    case UIVM_STRENGTH_THRESHOLD:   lcd.print("00");  break;
    case UIVM_STRENGTH_DIFFICULTY:  lcd.print("0%");  break;
    case UIVM_STRENGTH_DURATION:    lcd.print("s");   break;
  }
}//end UISystem::showVar

/* getState()
 *  This function is called from UISystem::loop() , MasterSystem::heartbeat()
 *  1. Returns that UI state
*/
byte UISystem::getState() {
  return state;
} // end getState

/* UISystem::enterState
 *  This function is called from UISystem::goStrengthPosthit , Accustat::heartbeat() , MasterSystem::accustatEnteringPosthit()
 *  called in UISystem::loop() 
 *  1. Function is passed a new state and updates the screen to display new state
 *  2. 
*/
void UISystem::enterState(byte newState) {
  DEBUG_PRINT_S(" UIS->");
  cur_var = BAD_VARNUM;
  sleep.wakeup();
  /* if sleep.wakeup(); was moved into each state, we could possibly add a uisleepstate
   *  This would allow for the screen to display that it is sleep, with the resevoir pressure
   *  and we could probably turn off the inverter
  */
//  Serial.println("UISystem::enterState is forcing wakeup ");
  lcd.display();
  lcd.clear();
  //Serial.print("UI enterState: "); Serial.println(newState);
  //if(sleep.getState() == SSS_AWAKE){
    switch (state = newState) {
      case UIS_TOWING:
      inverter.neededByDumpValve (false);
        DEBUG_PRINT_S("TOWING");
        lcd.print("=== TOWING MODE ===");
        lcd.setCursor(0, 1);  lcd.print("Reservoir Pressure:");
        break;
  
      case UIS_SCRUM_POWER:
      inverter.neededByDumpValve (false);
        DEBUG_PRINT_S("POWER");
        lcd.print("** POWER DRIVING **");
        lcd.setCursor(3, 1);  lcd.print("Threshold:");
        break;
  
      case UIS_SCRUM_INDIVIDUAL:
      inverter.neededByDumpValve (false);
        DEBUG_PRINT_S("INDIVIDUAL");
        lcd.print("++++ INDIVIDUAL ++++");
        lcd.setCursor(0, 1);
        lcd.print("++++  TRAINING  ++++");
        break;
  
      case UIS_SCRUM_STRENGTH:
        halSetPushbackDumpValve(LOW);
        inverter.neededByDumpValve(true);  // inverter will be needed soon to power dump valve
        DEBUG_PRINT_S("STRENGTH");
        lcd.print(" STRENGTH TRAINING ");
        lcd.setCursor(2, 1);  lcd.print("Threshold:");
        lcd.setCursor(2, 2);  lcd.print("Difficulty:");
        lcd.setCursor(2, 3);  lcd.print("Duration:");
        break;
  
  
  // Below is when the machine is pushing back
      case UIS_SCRUM_STRENGTH_CHARGE:
        initcharge.enable(false);  // lock in Init Charge pressure
        halSetPushbackDumpValve(HIGH);
        DEBUG_PRINT_S("STRENGTH-CHARGE");
        lcd.print(" STRENGTH TRAINING ");
        lcd.setCursor(6, 2);
        lcd.print("CHARGING");
        break;
  
      case UIS_SCRUM_STRENGTH_POSTHIT:
        halSetPushbackDumpValve(LOW);
        inverter.neededByDumpValve(false);
        DEBUG_PRINT_S("STRENGTH-POSTHIT\n");
        lcd.print(" STRENGTH TRAINING ");
        if (strengthPosthitCode == UISPH_SUCCESS) {
          lcd.setCursor(6, 2);
          lcd.print("SUCCESS");
        } else {
          // shutdown
          lcd.setCursor(6, 1);
          lcd.print("SHUTDOWN");
          lcd.setCursor(6, 2);
          switch (strengthPosthitCode) {
            case UISPH_TOO_HIGH:  lcd.print("TOO HIGH");  break;
            case UISPH_TOO_FAST:  lcd.print("TOO FAST");  break;
            case UISPH_TOO_MUCH:  lcd.print("TOO MUCH");  break;
            default:
              lcd.print("?? ");
              lcd.print(strengthPosthitCode);
              lcd.print(" ??");
              break;
          }
          lcd.setCursor(8, 3);
          lcd.print(strengthPosthitValue);
        }
        break;
    } // switch (state = newState)
//  }else{// closes if(sleep.getState)
//      lcd.print("=== SLEEPING ===");
//      lcd.setCursor(0, 1);  lcd.print("Reservoir Pressure:");
//      showVar(UIVN_TOWING_RESPRESS);  
//  }
  
  master.UIModeChanged(state);
} // end UISystem::enterState

/* goStrengthPosthit
 *  Called from MasterSystem::loop() 
 *  1. This function assigns a code, and value that will be used in other functions
 *  2. 
*/
void UISystem::goStrengthPosthit(byte SPH_code, int val) {

//  Serial.print("goStrengthPosthit started. SPH_code: "); Serial.println(SPH_code); Serial.print("val: "); Serial.println(val);
//  Serial.println("wainting for debug button");
//  //while(pulseIn(ioTight_ball_sonar, HIGH) > 300); // debug, waits untill high by user debug button

  strengthPosthitCode  = SPH_code;
  strengthPosthitValue = val;
  DEBUG_PRINT_S(" UIgSPH:");
  switch (SPH_code) {
    case UISPH_TOO_HIGH:  DEBUG_PRINT_S("H,");  break;
    case UISPH_TOO_FAST:  DEBUG_PRINT_S("F,");  break;
    case UISPH_TOO_MUCH:  DEBUG_PRINT_S("M,");  break;
    case UISPH_SUCCESS:   DEBUG_PRINT_S("SUCC,");  break;
    default:
      DEBUG_PRINT_I(SPH_code);
      DEBUG_PRINT_S(",");
      break;
  }
  DEBUG_PRINT_I(val);
  enterState(UIS_SCRUM_STRENGTH_POSTHIT);
} // end goStrengthPosthit

/* changeVar
 *  Called from UISystem::loop() 
 *  1. this deletes old variable, and prints the new variables that can be modified
*/
void UISystem::changeVar(byte newVar) {
  // erase old "current variable" chevrons
  if (cur_var != BAD_VARNUM) {
    lcd.setCursor(cols[cur_var] - CHEVRON_LEFT,  rows[cur_var]);  lcd.print(" ");
    lcd.setCursor(cols[cur_var] + CHEVRON_RIGHT, rows[cur_var]);  lcd.print(" ");
  }

  cur_var = newVar;

  // draw new "current variable" chevrons
  if (cur_var != BAD_VARNUM) {
    lcd.setCursor(cols[cur_var] - CHEVRON_LEFT,  rows[cur_var]);  lcd.print(">");
    lcd.setCursor(cols[cur_var] + CHEVRON_RIGHT, rows[cur_var]);  lcd.print("<");
  }
} // end changeVar
