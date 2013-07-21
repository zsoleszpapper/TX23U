#ifndef TX23U_h
#define TX23U_h

#include "Arduino.h"

//#define TX23U_DBG

class TX23U {

 private:
  static const byte MAXSAMPLES = 123;
  static const byte MAXDATA = 41;
  static const int samplingDelay = 400;

  int TX20_TxD = 2;

  boolean samples[MAXSAMPLES];
  boolean data[MAXDATA];
  byte pointer = 0;

  void changeAlert();
  int waitForChangeInterrupt(unsigned long timeout);
  void initializeBuffers();
  int requestData();
  void busyLoopSampling();
  void downSample();
  boolean validateData();
  void processData();

 public:
  TX23U(int);
  void printRawData();
  boolean getData();

  byte direction = 0;
  int speed = 0;

  const char DIRECTIONS[16][4] = {
   "N",
   "NNE",
   "NE",
   "ENE",
   "E",
   "ESE",
   "SE",
   "SSE",
   "S",
   "SSW",
   "SW",
   "WSW",
   "W",
   "WNW",
   "NW",
   "NNW"
  };

};

#endif
