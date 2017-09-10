#include <MLEDScroll.h>

MLEDScroll matrix;

void intens() {
  for(int i=1;i<=8;i++){
    matrix.setIntensity(i);
    delay(15);
  }
  delay(250);
  for (int i=9;i>=0;i--) {
    matrix.setIntensity(i);
    delay(35);
  }
}

void setup() {
  matrix.begin();
  //matrix.flip=false;
}

void loop() {
  matrix.character("L");
  intens();
  matrix.character("e");
  intens();
  matrix.character("t");
  intens();
  matrix.character("t");
  intens();
  matrix.character("e");
  intens();
  matrix.character("r");
  intens();
  matrix.character("s");
  intens();
  matrix.character(" ");
  intens();
}
