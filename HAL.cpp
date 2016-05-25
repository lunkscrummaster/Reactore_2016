#include "HAL.h"

#include "pinDefs.h"

// Sonar accuracy might be improved by using Input Capture hardware,
// see: https://gist.github.com/mpflaga/4404996

// detects ball if ball-in sonars are less than this (micros)
#define BALLIN_TRIP  5000//chagned from 3000 to 5000, should trip at anything less than 50cm


////////// HARDWARE-SPECIFIC SONAR ROUTINES ////////////////////////////////////////

//int hal_ReadSonarHCSR04(byte tries, byte trigPin, byte sensePin) {
//  int durationMicros = 0;
//  while (tries-- > 0) {
//    digitalWrite(trigPin, LOW);   delayMicroseconds(2);
//    digitalWrite(trigPin, HIGH);  delayMicroseconds(10);
//    digitalWrite(trigPin, LOW);
//    durationMicros = pulseIn(sensePin, HIGH, 10000);
//    if (durationMicros > 0)
//      break;
//  }
//  return durationMicros;
//}

// code based on http://www.seeedstudio.com/wiki/Ultra_Sonic_range_measurement_module
//int hal_ReadSonarSEN136B5B(byte tries, byte pin) {
//  int durationMicros = 0;
//  while (tries-- > 0) {
//    pinMode(pin, OUTPUT);
//    digitalWrite(pin, LOW);   delayMicroseconds(2);
//    digitalWrite(pin, HIGH);  delayMicroseconds(5);
//    digitalWrite(pin, LOW);
//
//    pinMode(pin, INPUT);
//    durationMicros = pulseIn(pin, HIGH, 10000);
//    if (durationMicros > 0)
//      break;
//  }
//  return durationMicros;
//}

//int hal_ReadSonarMB1020(byte tries, byte pin) {
//  int durationMicros = 0;
//  while (tries-- > 0) {
//    durationMicros = pulseIn(pin, HIGH, 10000);
//    if (durationMicros > 0)
//      break;
//  }
//  return durationMicros;
//}

/* hal_ReadSonarMB7360
 *  This function is called from halIsBallIn (bellow) , halReadSonar_OutriggerLoose , halReadSonar_OutriggerTight , halReadSonar_Pushback
 *  1. This function takes the number of tries that the sonar should read, and returns the value if it reads one
 *  2. a conversion is completed in order to return the correct value
*/
int hal_ReadSonarMB7360(byte tries, byte pin) {
  unsigned long durationMicros = 0;
  while (tries-- > 0) {
    durationMicros = pulseIn(pin, HIGH, 100000);
     
    if (durationMicros > 0) //if get a good reading, then break out
      break;
  }// end while

  // MB7360 counts one microsec per millimetre distance;
  // simpler sonars count about 5.8 microsecs per millimetre distance (the speed of sound),
  // so convert the MB7360 microsecs into a time-of-flight microsec count
  //durationMicros = durationMicros * 58 / 10;
  return durationMicros; //< 32768L ? durationMicros : 32767;
} // end hal_ReadSonarMB7360

////////// 'GLOBAL' HARDWARE ABSTRACTION ROUTINES ////////////////////////////////////////
//HAL dirvers below, setup hardware; pinouts
/* halSetup()
 *  The function is called from setup() in the main page
 *  1. The first part creates an arrary, and initalizes the pins to low
*/
void halSetup() {
  byte outPins[] = {
    LOW,  oPushbackArmDownPin,
    LOW,  oPushbackArmUpPin,

    LOW,  oAirSpringLockout,
    LOW,  oChargeAlternatePin,

    LOW,  oReservoirLockout,
    LOW,  oUnloaderPin,

    LOW,  oInitialChargeUp,
    LOW,  oInitialChargeDown,

    LOW,  oInverterPin,
    HIGH,  oBatteryLink,

    LOW,  oSuccess,
    LOW,  oBeeper,

    LOW,  oCompressorPin,
    LOW,  oPushbackDumpValve,

    LOW,  oDisplayPowerPin,
 //   LOW,  oPushback_sonar_trigger,

    LOW,  oLEDClear,
    LOW,  oLEDData,
    LOW,  oLEDClock,

    LOW,  oOutriggerTightUp,
    LOW,  oOutriggerTightDown,

    LOW,  oOutriggerLooseUp,
    LOW,  oOutriggerLooseDown,

    //    LOW,  oOutriggerTightSonarTrigger,
    //    LOW,  oOutriggerLooseSonarTrigger,

    0xff
  };
  for (byte i = 0; true; i += 2) {
    byte st = outPins[i];
    if (st >= 0xff)
      break;                        //breaks at end of array signified by 0xff
    byte pin = outPins[i + 1];
    digitalWrite(pin, st);
    pinMode(pin, OUTPUT);
  } //end for
} //end halSetup()

// reads sonar and detects ball (TRUE if ball, FALSE if no ball)
/* halIsBallIn
 *  This function is called from Accustat::heartbeat() 
 *  1. This function reads the sonar value, and returns value
 *  #define BALLIN_TRIP  5000//chagned from 3000 to 5000, should trip at anything less than 
 *  BALLIN_TRIP should be tested and changed depending on wanted trip level
*/
boolean halIsBallIn(byte pin) {
  int sonar = hal_ReadSonarMB7360(2, pin);
 // Serial.print("sonar value: ");Serial.print(sonar);Serial.print(" Pin #: ");Serial.print(pin);
 // Serial.println(" ballin? ");
  return sonar > 0 && sonar < BALLIN_TRIP;// sonar reads value > 0 and less than or equal to ???
/**************************************************************************************************************
 * what value does that BALLIN_TRIP need to be? 
 * Remember the sonar reads 29~30cm when detecting 30cm or less objects
 *****************************************************************************************************/
} // end halIsBallIn

/* halIsTowScrumSwitchInTowing
 *  This function is called from MasterSystem::heartbeat()
 *  1. This function reads the aiScrumPin  A7 Determines if scrum(high) or tow(low) based on the position of airswitch at back of vehicle. 
 * Why does this retrun < 500 ?????????************************
*/
boolean halIsTowScrumSwitchInTowing() {
  return analogRead(aiScrumPin) < 500;
} // end halIsTowScrumSwitchInTowing

/* halSetInitialChargeUpDown
 *  This function is called from InitialChargeSystem::enterState 
 *  1. Takes a value, and writes charge up and down pins high or low depending on previous state
 *  2. If dir == 0, both are written low. therefor does not charge up or down
*/
void halSetInitialChargeUpDown(int dir) {
  digitalWrite(oInitialChargeDown, dir < 0 ? HIGH : LOW);
  digitalWrite(oInitialChargeUp,   dir > 0 ? HIGH : LOW);
} // end halSetInitialChargeUpDown

/* halSetPushbackUpDown
 *  This function is called from PushbackSystem::enterState  
 *  1. Writes the pushback arm to raise or lower depending on the previous state
*/
void halSetPushbackUpDown(int dir) {
  digitalWrite(oPushbackArmDownPin, dir < 0 ? HIGH : LOW);
  digitalWrite(oPushbackArmUpPin,   dir > 0 ? HIGH : LOW);
} // end halSetPushbackUpDown

/* halSetPushbackDumpValve
 *  This function is called from UISystem::enterState 
 *  1. Writes the dump valve to either high, or low, depending on what is passed in
*/
void halSetPushbackDumpValve(byte level) {
  digitalWrite(oPushbackDumpValve, level);
} // end halSetPushbackDumpValve

/* halReadSonar_OutriggerLoose
 *  This function is called from OutriggerSystem::loop() , OutriggerSystem::heartbeat(void) 
 *  1. Returns the value of the outriggerloose sonar
*/
int halReadSonar_OutriggerLoose(byte tries) {
  return hal_ReadSonarMB7360(tries, ioOutriggerLooseSonar);
} // end halReadSonar_OutriggerLoose

/* halReadSonar_OutriggerTight
 *  This function is called from OutriggerSystem::loop() , OutriggerSystem::heartbeat(void)
 *  1. Returns the value of the outriggertight sonar
*/
int halReadSonar_OutriggerTight(byte tries) {
  return hal_ReadSonarMB7360(tries, ioOutriggerTightSonar);
} // end halReadSonar_OutriggerTight


/* halReadSonar_Pushback(byte)
 *  This function is called from MasterSystem::loop() , PushbackSystem::heartbeat()
 *  1. Returns the value of the pushback sonar
*/
int halReadSonar_Pushback(byte tries) {
//  return hal_ReadSonarHCSR04(tries, oPushback_sonar_trigger, iPushback_sonar_echo);
  return hal_ReadSonarMB7360(tries, iPushback_sonar_echo);
} // end halReadSonar_Pushback


