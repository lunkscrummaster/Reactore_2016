#include "CIS.h"
#include "SS.h"
#include "HAL.h"

#include "pinDefs.h"  // inverterPin, inverterOnPin

////////////////////////////// CompressorSystem //////////////////////////////

// CompressorSystem states
#define CSS_OFF       0 //compressor system is curreently off
#define CSS_STARTING  1  // waiting for inverter on and unloader valve timeout
#define CSS_ON        2 //compressor system is on
#define CSS_STOPPING  3  // waiting for unloader valve timeout

CompressorSystem::CompressorSystem() {
  pvTankPressMin = 700;
  pvTankPressMax = 940;
}
/* CompressorSystem::heartbeat
 *  This is called from the heartbeat() in the main page
 *  1. Reads the reservoir pressure
 *  2. Fills tank when it is too low, enters starting state
 *  3. Waits for one loop through, then enters the on state
 *  4. Once tank is filled to correct pressure, enter stopping state
 *  5. Waits for on loop through, then enters the off state
*/
void CompressorSystem::heartbeat(void)
{
  Serial.print("Compressor System Heartbeat Started, State: "); Serial.println(comp.state);
 
  int reservoirPressure = analogRead(aiReservoirPin);

  Serial.print("resevoirPressure: ");Serial.println(reservoirPressure);
  Serial.println("waiting for debug button");
  while(analogRead(aitestdebugtrigger) > 300);  Serial.println("continue"); // debug, waits untill pin 17 written high by user debug button
  
  switch (state) {
    case CSS_OFF:
      if (reservoirPressure < pvTankPressMin) { //if the compressor is off, and the tank is to low, turn it on
        inverter.neededByCompressor(true); 
        digitalWrite(oUnloaderPin, HIGH); //dump pressure for 2 heart beats before compressor is truned on
        pvUnloaderTimeout = 2;    // number of heartbeat intervals to wait for unloader
        enterState(CSS_STARTING); //compressor system is now starting
      } else
        digitalWrite(oCompressorPin, LOW); //turn off compressor
      break;

    case CSS_STARTING:
      if (pvUnloaderTimeout > 1) {
        pvUnloaderTimeout--;  // keep waiting
      } else if (digitalRead(iInverterOnPin)) {
        digitalWrite(oUnloaderPin, LOW);        // unloader has timed out so close it
        digitalWrite(oCompressorPin, HIGH);
        enterState(CSS_ON); //the compressor is now on
      }
      break;

    case CSS_ON:
      if (reservoirPressure > pvTankPressMax) {
        digitalWrite(oCompressorPin, LOW); //turn off if tank is pressurized
        inverter.neededByCompressor(false); //compressor is no longer needed by inverter
        pvUnloaderTimeout = 2;    // number of heartbeat intervals to wait for unloader
        enterState(CSS_STOPPING); //enter the stopping state
      } else
        digitalWrite(oCompressorPin, HIGH); //if tank pressure is not achieved, keep  compressor on
      break;

    case CSS_STOPPING:
      if (pvUnloaderTimeout > 1) {
        pvUnloaderTimeout--;  // keep waiting
      } else {
        // unloader has timed out so close it
        digitalWrite(oUnloaderPin, LOW); 
        enterState(CSS_OFF); //compressor is now off
      }
      break;
  } //end switch (state)

#ifdef DEBUG
  if (debugFlagCS) {
    DEBUG_PRINT_S(" CS: resP=");
    DEBUG_PRINT_I(reservoirPressure);
    DEBUG_PRINT_S(" st=");
    switch (state) {
      case CSS_OFF:       DEBUG_PRINT_S("OFF"     );  break;
      case CSS_STARTING:  DEBUG_PRINT_S("STARTING");  break;
      case CSS_ON:        DEBUG_PRINT_S("ON"      );  break;
      case CSS_STOPPING:  DEBUG_PRINT_S("STOPPING");  break;
    }
  }
#endif
} //end CompressorSystem::heartbeat


/* CompressorSystem::enterState(byte)
 *  This function is called in CompressorSystem::heartbeat above ^^
 *  1. This just sets state to the new state, heartbeat handles the rest of switching compressor states
*/
void CompressorSystem::enterState(byte newState) {
  state = newState;
  Serial.print("Compressor System enterState: "); Serial.println(newState);
} // end CompressorSystem::enterState

////////////////////////////// InverterSystem //////////////////////////////

InverterSystem::InverterSystem() 
{
  digitalWrite(oInverterPin, LOW);
  pinMode(oInverterPin, OUTPUT);
  pinMode(iInverterOnPin, INPUT_PULLUP);
}

/* InverterSystem::heartbeat()
 *  This function is called from heartbeat() in the main page
 *  1. The heartbeat turns the inverter on or off when it is needed
 *  I think the inverter works like a button that turns a light on and off.
 *  Push the button and it turns the light(inverter) on.
 *  Push the button again, and it turns the light(inverter) OFF
*/
void InverterSystem::heartbeat() {
  Serial.println("Inverter System heartbeat start ");
  
  boolean enabled = nbCompressor || nbDumpValve; //either these need inverter?

  byte ac = digitalRead(iInverterOnPin);
  // iInverterOnPin: High when inverter on
  // iInverterOnPin: Low when inverter off

  Serial.print("iInverterOnPin: "); Serial.println(ac);
  Serial.print("enabled: "); Serial.println(enabled);
  Serial.println("waiting for debug button");
  while(analogRead(aitestdebugtrigger) > 300);  Serial.println("continue"); // debug, waits untill high by user debug button
  
  byte st;
  if (!enabled && !ac)  st = 0;
  if ( enabled && !ac)  st = 1;
  if ( enabled &&  ac)  st = 2;
  if (!enabled &&  ac)  st = 3;

  switch (st) {
    case 1:  // starting
      digitalWrite(oBatteryLink, HIGH); Serial.println("Inverter starting");    // link the batteries while inverter is running
    case 3:  // stopping
      if (! digitalRead(oInverterPin)) // ???? lol wat
        digitalWrite(oInverterPin, HIGH); Serial.println("Inverter stopping");  // begins pressing button on inverter
      break;
// ???? whenever the system is awake, the batteries should be linked
    case 0:  // stopped
     // ****  digitalWrite(oBatteryLink, LOW); Serial.println("Inverter stopped");   // unlink the batteries while inverter not running
    case 2:  // running
      if (digitalRead(oInverterPin))
        digitalWrite(oInverterPin, LOW);  Serial.println(" Inverter running");// stops pressing button when inverter has started/stopped
      break;
  } //end switch

#ifdef DEBUG
  //    DEBUG_PRINT_S(" INS: Sens=");
  //    DEBUG_PRINT_I(digitalRead(iInverterOnPin));
  //    DEBUG_PRINT_S(" Btn=");
  //    DEBUG_PRINT_I(digitalRead(oInverterPin));22
  //    DEBUG_PRINT_S(" Ena=");
  //    DEBUG_PRINT_I(enable);
  //    DEBUG_PRINT_S(" ");
  //    switch (st) {
  //      case 0:  DEBUG_PRINT_S("stopped" );  break;
  //      case 1:  DEBUG_PRINT_S("starting");  break;
  //      case 2:  DEBUG_PRINT_S("running" );  break;
  //      case 3:  DEBUG_PRINT_S("stopping");  break;
  //    }
#endif
} // end InverterSystem::heartbeat()


/* InverterSystem::neededByCompressor
 *  This function is called in CompressorSystem::heartbeat 
 *  1. nbCompressor is a boolean, and just sets that the flag to true or false
*/
void InverterSystem::neededByCompressor(boolean en) {
  nbCompressor = en;
  sleep.wakeup(); //wake up system ****
  /* **** Changes made May 20, 2016 By t,z,k
   *  Whenever the compressor is needed while asleep, the system will wake up
   *  This will allow for the batteries to be linked
  */
} //end InverterSystem::neededByCompressor

/* InverterSystem::neededByDumpValve
 *  This is called by UISystem::enterState  
 *  1. nbDumpValve is a boolean, and just sets the flag to true or false
*/
void InverterSystem::neededByDumpValve(boolean en) {
  nbDumpValve = en;
} //end InverterSystem::neededByDumpValve




