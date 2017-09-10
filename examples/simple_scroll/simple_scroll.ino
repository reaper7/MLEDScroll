#include <MLEDScroll.h>

MLEDScroll matrix;

void setup() {
  matrix.begin();
  //matrix.flip=false;
  matrix.message("Scroll ");
}

void loop() {
  matrix.scroll();
}
