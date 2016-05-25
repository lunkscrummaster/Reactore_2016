#include <LiquidCrystal.h>
#include <Timer.h>  // https://github.com/JChristensen/Timer
#include <TimerOne.h>
#include "Debug.h"


// battery charge circuit alternates every 'CHARGE_ALTERNATE_MINUTES'
#define CHARGE_ALTERNATE_MINUTES   60
#define MILLIS_MAX 4294967295
#define MICROS_MAX 4294967295
#define TRAVEL_TIME   1000
#define sonarTimer Timer1
/* Hi Kevin. Hope you are doing well. If you want to change the TRAVEL_TIME, which is the length of time
    the sled will be pushed after there was a successful push.
    1000 = 1 second
    2000 = 2 seconds etc...
*/

#include "AS.h"
#include "CIS.h"
#include "ICS.h"
#include "MAS.h"
#include "ORS.h"
#include "PBS.h"
#include "SS.h"
#include "UIS.h"

#include "HAL.h"

#include "pinDefs.h"  // debugging

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);


////////////////////////////// debugging //////////////////////////////

#include "Debug.h"

#ifdef DEBUG  // DEBUGGING

#ifdef DEBUG_ORS
boolean debugORSSelect;
#endif

boolean debugFlagAS    = false;
boolean debugFlagCS    = false;
boolean debugFlagICS   = false;
boolean debugFlagPBS   = false;
boolean debugFlagReady = true;   // shows progress of Ready sequence
boolean debugFlagSleep = true;
boolean debugFlagSonar = false;

void debugSetup() {
  Serial.begin(9600);
  Serial.println("==================================================");
}

void debugPrintS(const char* s)  {
  Serial.print(s);
}
void debugPrintI(int i) {
  Serial.print(i);
}

int debugRead() {
  return Serial.read();
}

#endif


////////////////////////////// main program //////////////////////////////
// REFER
MasterSystem         master;

Accustat             accustat;
CompressorSystem     comp;
InitialChargeSystem  initcharge;
InverterSystem       inverter;
OutriggerSystem      outriggers;    // handles both outriggers
PushbackSystem       pushback;
SleepSystem          sleep;
UISystem             ui(lcd);


Timer heartbeatTimer;

//Timer sonarTimer;


//============================== Charge Alternate ==============================

Timer chargeAlternateTimer;

void chargeAlternateCallback() {
  digitalWrite(oChargeAlternatePin, ! digitalRead(oChargeAlternatePin));
}


//============================== heartbeat ==============================

#ifdef DEBUG
extern void beep(byte count);  // AS.cpp
#endif

/* heartbeat()
    This is the main heartbeat of the system. All other heart beats are called from here.
*/
void heartbeat() {
  // Serial.println("Enter Heartbeat");
#ifdef DEBUG
  switch (debugRead()) {
    //  case '0':  digitalWrite(oSuccess, LOW);   break;
    //  case '1':  digitalWrite(oSuccess, HIGH);  break;

    case 'a':  debugFlagAS    = ! debugFlagAS;     break;
    case 'b':  beep(2);  break;  // to test beeper
    case 'c':  debugFlagCS    = ! debugFlagCS;     break;
    case 'i':  debugFlagICS   = ! debugFlagICS;    break;
    case 'p':  debugFlagPBS   = ! debugFlagPBS;    break;
    case 'r':  debugFlagReady = ! debugFlagReady;  break;
    case 's':  debugFlagSonar = ! debugFlagSonar;  break;
  }
#endif

  accustat.heartbeat();     //AS.cpp
  comp.heartbeat();         //CIS.cpp comp = compressor system
  initcharge.heartbeat();   //ICS.cpp initial charge system
  inverter.heartbeat();     //CIS.cpp inverter = inverter system
  master.heartbeat();       // MAS.cpp
  outriggers.heartbeat();   // ORS.cpp
  pushback.heartbeat();     // PBS.cpp
  sleep.heartbeat();        // SS.cpp
  ui.heartbeat();           // UIS.cpp

#ifdef DEBUG
  //  DEBUG_PRINT_S(" loose:");
  //  if (halIsBallIn(ioLoose_ball_sonar))
  //        DEBUG_PRINT_S(" BALL  ");
  //  else  DEBUG_PRINT_S("no-ball");

  //  DEBUG_PRINT_S(" tight:");
  //  if (halIsBallIn(ioTight_ball_sonar))
  //        DEBUG_PRINT_S(" BALL  ");
  //  else  DEBUG_PRINT_S("no-ball");

  //  int lim = debugSonar;
  //  if (lim > 999)  lim = 999;
  //  for (int i = 0; i < lim; i += 30)
  //    DEBUG_PRINT_S(" ");
  //  DEBUG_PRINT_I(debugSonar);
#endif

  //  if (pushback.isReady()) {
  //    transferSend.outPushback = ui.getVar(UISVN_POWER_CURRENT);
  //    ET_Send.sendData();
  //    DEBUG_PRINT_S(" sent-value=");
  //    DEBUG_PRINT_I(transferSend.outPushback);
  //  }

  //  DEBUG_PRINT_S(" rlo=");  DEBUG_PRINT_I(digitalRead(oReservoirLockout));
  DEBUG_PRINT_S(" #");
  DEBUG_PRINT_I(millis());
  DEBUG_PRINT_S("\n");
}


//============================== setup ==============================
/* setup()
    This sets up the program and the board.
*/
void setup() {
#ifdef DEBUG
  debugSetup();
#endif
  for (int i = 0; i < AVE_ARRAY_SIZE; i++) {
    master.pushbackSonar[i] = analogRead(aiPushbackSonar);
  }

  sonarTimer.initialize(100000);

  sonarTimer.attachInterrupt(sonarISR);

  halSetup(); //sets up the input and output pins used the board

  Serial.begin(9600); //sets up serial communication

  lcd.begin(20, 4); //sets up screen

  heartbeatTimer.every(1000L / HEARTBEATS_PER_SECOND, heartbeat); //calls heartbeat function when timer goes off.

  chargeAlternateTimer.every(CHARGE_ALTERNATE_MINUTES * 60L * 1000, chargeAlternateCallback);

  DEBUG_PRINT_S("Setup done\n");

  // Serial.println("Setup Complete");
}



//============================== loop ==============================
/* loop()
    This is the main loop of the program. After setup, code runs this loop.
*/

void loop() {

  //  Serial.println("Main Loop Started");
  //  Serial.println("waiting for debug button");
  //  Serial.print("This is the initial accustat mode"); Serial.println(accustat.returnmode());
  //
  //  //while(pulseIn(ioTight_ball_sonar, HIGH) > 300);  Serial.println("continue"); // debug, waits untill pin 34 written high by user debug button
  //
  long mofoStartLoop = millis();

  ui.loop();          // call debouncer frequently

  accustat.loop();    // to average pushback-arm pressure readings

  outriggers.loop();  // fast update of outrigger balancing system
  /* Code added May 21 by Trevor and Zach
      The code below is used for the successtimer which is what is used to determine how long the
      sled can be pushed after a success. Code handles rollovers after approx 50days
  */
  if (master.successStartTime > 0) {
    long currentMillis = millis() - master.successStartTime;
    if (currentMillis < 0)
      currentMillis += MILLIS_MAX; // rollover
    if (currentMillis > TRAVEL_TIME) {
      digitalWrite(oSuccess, LOW);
      master.successStartTime = 0;
    }

  }
  master.loop();      // during Strength Charge phase



#ifdef DEBUG_ORS
  outriggers.testLoop();
#endif

  heartbeatTimer.update();

  chargeAlternateTimer.update();

  Serial.print("  ************************ time for main loop:    "); Serial.println(millis() - mofoStartLoop);

} // end loop

void sonarISR() {                 //****added to constanty read pushback sonar. can add master shutdown control here later.
  for (int i = 0; i < AVE_ARRAY_SIZE; i++) {
    master.pushbackSonar[i] = master.pushbackSonar[i + 1];
  }
  master.pushbackSonar[AVE_ARRAY_SIZE - 1] = analogRead(aiPushbackSonar) * 5;

  if (pushback.readyRaiseTo <= master.pushbackSonar[AVE_ARRAY_SIZE - 1] &&
      pushback.getState()   == PBS_READY2_RAISING                       &&
      sleep.getState()      == SSS_AWAKE) {

    // start settling
    pushback.enterState(PBS_READY3_SETTLING);
    //pushback.heartbeat();
  }
  //    Serial.println("Sonar ISR Finished");
  //    if(analogRead(aiPushbackSonar)  >= max analog value)
  //        while(1)

}

