#ifndef CompressorInverterSystem_h
#define CompressorInverterSystem_h

#include "Arduino.h"
#include "Debug.h"


class CompressorSystem {
  public:
    CompressorSystem();
    void heartbeat(void);

  private:
    byte state;
    void enterState(byte newState);

    // reservoir pressure setpoints
    int  pvTankPressMin;
    int  pvTankPressMax;

    byte pvUnloaderTimeout;
};


class InverterSystem {
  public:
    InverterSystem();
    void heartbeat();
    void neededByCompressor(boolean en);  // compressor requests inverter on/off
    void neededByDumpValve (boolean en);  // compressor requests inverter on/off

  private:
    boolean nbCompressor, nbDumpValve;
};

#endif
