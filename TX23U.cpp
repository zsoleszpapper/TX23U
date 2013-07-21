#include "TX23U.h"

TX23U::TX23U(int pin) {
  TX20_TxD = pin;
  pinMode(TX20_TxD, INPUT);
  digitalWrite(TX20_TxD, HIGH);
}

int TX23U::waitForChangeInterrupt(unsigned long timeout) {
  unsigned long deadline;
  boolean old = digitalRead(TX20_TxD);
  deadline = millis();
  deadline += timeout;  //Can give fake timeouts if this command overflows deadline.
  while (millis() < deadline) {
    if (old != digitalRead(TX20_TxD)) { return 1; }
  }
  return 0;
}

void TX23U::initializeBuffers() {
#ifdef TX23U_DBG
  Serial.println("initializeBuffers()");
#endif
  for(pointer = 0; pointer < MAXSAMPLES; pointer++) {
    samples[pointer] = false;
  }
  for(pointer = 0; pointer < MAXDATA; pointer++) {
    data[pointer] = false;
  }
  pointer = 0;
}

int TX23U::requestData() {
#ifdef TX23U_DBG
  Serial.println("requestData()");
#endif
  pinMode(TX20_TxD, OUTPUT); // pin to output
  digitalWrite(TX20_TxD, HIGH); delay(1); digitalWrite(TX20_TxD, LOW); // pull reset signal start

  delay(500); //wait reset signal length

  pinMode(TX20_TxD, INPUT); //pin to input, reset signal end
  digitalWrite(TX20_TxD, HIGH);

  delay(11); // wait 1-2ms high, + half of the low period, approx 9ms = 11ms

  if (waitForChangeInterrupt(2000)==0) { return 0; } // wait for rising edge of the start frame with timeout
  return 1;
}


void TX23U::busyLoopSampling() {
  for(pointer=0;pointer<MAXSAMPLES;pointer++) {
    if (digitalRead(TX20_TxD)) {
      samples[pointer] = 1;
    } else {
      samples[pointer] = 0;
    }
    delayMicroseconds(samplingDelay);
  }
}

void TX23U::downSample() {
  int p;
  for(pointer=0;pointer<MAXDATA;pointer++) {
    p = pointer * 3;
    if (samples[p]+samples[p+1]+samples[p+2] > 1) {
      data[pointer] = 1;
    } else {
      data[pointer] = 0;
    }
  }
}

boolean TX23U::validateData() {
 int i;
#ifdef TX23U_DBG
 Serial.print("samplingDelay = ");
 Serial.println(samplingDelay);
#endif
 if (
  (data[0] == 1) and
  (data[1] == 1) and
  (data[2] == 0) and
  (data[3] == 1) and
  (data[4] == 1)
 ) {
#ifdef TX23U_DBG
     Serial.println("Fix header OK... ");
#endif
   } else {
#ifdef TX23U_DBG
     Serial.println("Fix header NOT OK... ");
#endif
     return false;
   }

 for(i=5;i<=20;i++) {
   if (data[i] != !data[i+20]) {
#ifdef TX23U_DBG
   if (i<=8) {
     Serial.println("Direction NOT OK... ");
   } else {
     Serial.println("Speed NOT OK... ");
   }
#endif
     return false;
   }
#ifdef TX23U_DBG
   if (i==8) { Serial.println("Direction OK... "); }
   if (i==20) { Serial.println("Speed OK... "); }
#endif
 }

 byte nibbles[5];
 for(i=0;i<5;i++) {
   nibbles[i]=0;
   nibbles[i] = data[i*4+8] << 3 | data[i*4+7] << 2 | data[i*4+6] << 1 | data[i*4+5];
 }

#ifdef TX23U_DBG
 Serial.print("Calculated checksum = "); Serial.println((nibbles[0]+nibbles[1]+nibbles[2]+nibbles[3])&0x0F);
 Serial.print("Received checksum   = "); Serial.println(nibbles[4]);
#endif
 if (nibbles[4] == ((nibbles[0]+nibbles[1]+nibbles[2]+nibbles[3])&0x0F)) {
#ifdef TX23U_DBG
   Serial.println("Checksum OK... ");
#endif
 } else {
#ifdef TX23U_DBG
   Serial.println("Checksum NOT OK... ");
#endif
   return false;
 }

 return true;
}

void TX23U::processData() {
#ifdef TX23U_DBG
  Serial.println("processData()");
#endif
  direction = data[8] << 3 | data[7] << 2 | data[6] << 1 | data[5];

  speed = 0;
  for(pointer=20;pointer>=9;pointer--) {
   speed |= data[pointer] << (pointer-9);
  }

#ifdef TX23U_DBG
 Serial.print("Direction: ");
 for(pointer=8;pointer>=5;pointer--) {
   if (data[pointer]) {
     Serial.print("1");
   } else {
     Serial.print("0");
   }
 }
 Serial.println();
 Serial.print("Speed: ");
 for(pointer=20;pointer>=9;pointer--) {
   if (data[pointer]) {
     Serial.print("1");
   } else {
     Serial.print("0");
   }
 }
 Serial.println();
#endif
}

void TX23U::printRawData() {
  for(pointer=0;pointer<MAXSAMPLES;pointer++) {
    if (samples[pointer]) {
      Serial.print("1");
    } else {
      Serial.print("0");
    }
  }
  Serial.println();
  for(pointer=0;pointer<MAXDATA;pointer++) {
    if (data[pointer]) {
      Serial.print(" 1 ");
    } else {
      Serial.print(" 0 ");
    }
  }
  Serial.println();
}

boolean TX23U::getData() {
  initializeBuffers();
  if (requestData()==0) {
#ifdef TX23U_DBG
    Serial.println("TIMEOUT requestData!");
    printRawData();
#endif
    return false;
  }
  delayMicroseconds(samplingDelay/2);
  busyLoopSampling();
  downSample();
  if (!validateData()) {
#ifdef TX23U_DBG
    Serial.println("Validate data failed!");
    printRawData();
#endif
    return false;
  }
  processData();
  return true;
}


