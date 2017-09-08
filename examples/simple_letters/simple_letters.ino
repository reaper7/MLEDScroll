#include <MLEDScroll.h>

#if defined ( ESP8266 )
MLEDScroll matrix(0, 13, 14);
#elif defined ( ESP32 )
MLEDScroll matrix(0, 23, 18);
#endif

void intens() {
  for(int i=0;i<8;i++){
    matrix.display(i);
    delay(15);
  }
  delay(250);
  for (int i=7;i>0;i--) {
    matrix.display(i);
    delay(35);
  }
  matrix.clear();
  delay(35);
}

void setup() {
  matrix.begin();
  //matrix.flip=true;
}

void loop() {
  matrix.letter("L");
  intens();
  matrix.letter("e");
  intens();
  matrix.letter("t");
  intens();
  matrix.letter("t");
  intens();
  matrix.letter("e");
  intens();
  matrix.letter("r");
  intens();
  matrix.letter("s");
  intens();
  matrix.letter(" ");
  intens();
}
