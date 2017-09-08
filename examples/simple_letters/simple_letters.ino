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
