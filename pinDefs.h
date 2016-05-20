////////////////////////////// Debug Pins
//#define debugPin 17
// lcd 2,3,4,5,6,7
////////////////////////////// SONAR I/O PINS

// outrigger level sonars
//#define oOutriggerTightSonarTrigger       47
//#define oOutriggerLooseSonarTrigger       51
#define ioOutriggerTightSonar             47
#define ioOutriggerLooseSonar             49

// pushback arm sonar
//******** why is there two sonars below????****************//
#define oPushback_sonar_trigger           30 //don't worry about this pin
#define iPushback_sonar_echo              32 //use this as the PUSHBACK SONAR

// ball-in sonars
#define ioLoose_ball_sonar                34
#define ioTight_ball_sonar                51

////////////////////////////// OUTPUT PINS

#define oPushbackArmDownPin               38  //big arm spring
#define oPushbackArmUpPin                 40  

/* oAirSpringLocket
 *  LOW:  
 *  HIGH: When written high, cuts off the switch and allows air to be pumped into the spring
*/
#define oAirSpringLockout                 42        //bleeds air out of air springs, charges the air system, blocking vents for leveling
#define oChargeAlternatePin               44        // Charge alternating relay

#define oReservoirLockout                 46        // shut off to stop leaks, opens when written high
#define oUnloaderPin                      48        // unloads airpressure from compressor head (otherwise compressor won't start), 

#define oInitialChargeUp                  50        // increase Initial Charge, resevoir that will dump on players when the ball goes in, changes depending on the difficulty setting
#define oInverterPin                      52        // Inverter control

/*  When oBatteryLink is high, the batteries must link after being woken up
*/
#define oBatteryLink                      31        // HIGH ties batteries together
#define oInitialChargeDown                33        // decrease Initial Charge, decrease that initial charge resevoir of air

#define oSuccess                          35        // activates dump valve to release pressure on hydraulics to allow freewheel
#define oBeeper                           37        // beeper to alert new high reading

#define oCompressorPin                    23        // 110AC for Compressor
#define oPushbackDumpValve                25        // powervalve to dump Initial Charge tank into pushback arm spring

#define oDisplayPowerPin                  27        // Turns off large display when sleeping

// tighthead outrigger control (non-battery side)
/*  The two pins below, are written low the entire time the system is running.
 *   These machine only uses the loosehead outrigger (battery side) to balance the machine
 *   
*/
#define oOutriggerTightUp                 39
#define oOutriggerTightDown               41

// loosehead outrigger control (battery side)
#define oOutriggerLooseUp                 45
#define oOutriggerLooseDown               43

// LED display
#define oLEDClear                         22
#define oLEDClock                         24
#define oLEDData                          26


////////////////////////////// INPUT PINS

#define iInverterOnPin                    28

#define iButtonPinSet                      8        // SET BUTTON
#define iButtonPinDown                     9        // DOWN BUTTON
#define iButtonPinUp                      10        // UP BUTTON
#define iButtonPinMode                    11        // MENU BUTTON
//???? looking at how it's wired, pin 13 looks like it is wired to pin 12
// **** iTrailerPowerPin has been changed from pin 13, to pin 12. May 19, 2016
/* iTrailerPowerPin ????
 *  This function has not been implemented yet.
 *  When pulled low, truck is plugged in.
 *  We are in towing mode. 
 *  1. Keep the air lockout resevoir on
 *  2. Monitor in the compressor and air in the resevoir
 *  3. In order for it to compress air, link the batteries, check to see if the inverter is on. dump the compressor unloader valve
 *  and signal the relay to hook up the 110 to the compressor
 *  4. The trailer charges the batteries
 *  
*/
#define iTrailerPowerPin                  12        // ------------ pulled LOW when towing power on

#define aiAchievedPin                     A0        // senses the air pressure built up in the main pushback arm when pushed or powervalve fires records what is held  by team 
#define aiReservoirPin                    A1        // senses the air reservoir pressure 
#define aiScrumPin                        A7        // Determines if scrum(high) or tow(low) based on the position of airswitch at back of vehicle. 
#define aiPrechargePin                    A8        // PRESSURE IN PRECHARGE CYLINDER. 
#define aitestdebugtrigger                A3        //testing, delete when done ????
