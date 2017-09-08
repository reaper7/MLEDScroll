#include <MLEDScroll.h>

#if defined ( ESP8266 )
MLEDScroll matrix(0, 13, 14);
#elif defined ( ESP32 )
MLEDScroll matrix(0, 23, 18);
#endif

void setup() {
  matrix.begin();
  //matrix.flip=true;
}

void loop() {
  matrix.dot(0, 0, 1);
  matrix.dot(7, 0, 0);
  matrix.dot(0, 7, 0);
  matrix.dot(7, 7, 0);
  matrix.display();
  delay(500);

  matrix.dot(0, 0, 0);
  matrix.dot(7, 0, 1);
  matrix.dot(0, 7, 0);
  matrix.dot(7, 7, 0);
  matrix.display();
  delay(500);

  matrix.dot(0, 0, 0);
  matrix.dot(7, 0, 0);
  matrix.dot(0, 7, 1);
  matrix.dot(7, 7, 0);
  matrix.display();
  delay(500);
  
  matrix.dot(0, 0, 0);
  matrix.dot(7, 0, 0);
  matrix.dot(0, 7, 0);
  matrix.dot(7, 7, 1);
  matrix.display();
  delay(500);

  matrix.clear();
  delay(500);
}
