#include <MLEDScroll.h>

#if defined ( ESP8266 )
MLEDScroll matrix(0, 13, 14);
#elif defined ( ESP32 )
MLEDScroll matrix(0, 23, 18);
#endif

void setup() {
  matrix.begin();
  //matrix.flip=true;
  matrix.message("Scroll ");
}

void loop() {
  matrix.scroll();
}
