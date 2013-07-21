#include "TX23U.h"

// Wind sensors TxD pin is connected to Arduinos pin 2
TX23U WIND(2);

void setup() {
  Serial.begin(9600);
}

void loop() {
  delay(5000);
  if (WIND.getData()) {
    Serial.print("Direction: ");
    Serial.println(WIND.DIRECTIONS[WIND.direction]);
    Serial.print("Speed: ");
    Serial.println(WIND.speed);
  } else {
    Serial.println("ERROR");
  }
}
