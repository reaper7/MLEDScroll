#include <MLEDScroll.h>

//#define SERIALDEBUG

#if defined ( ESP8266 )

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFiMulti.h>

#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

ESP8266WiFiMulti wifiMulti;

#elif defined ( ESP32 )

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiMulti.h>

#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

WiFiMulti wifiMulti;

#endif

AsyncWebServer server(80);
MLEDScroll matrix;

const char *ssid = "YourWiFiSSID";
const char *password = "YourWiFiPASS";

String devicename = "TEXTSCROLL";

volatile uint8_t otaintens = 7;
volatile bool otaup = false;
volatile unsigned long otalast = 0;

volatile bool ipshow = false;

String outMsg = "";

bool scrollup = true;

#define MAXTEXTLEN  250

//------------------------------------------------------------------------------
String utf8toiso(String _txt) {
  uint8_t curpos = 0;
  uint8_t maxpos = _txt.length();
  char charMsg[maxpos+1];
  bool fnd = false;

  _txt.toCharArray(charMsg, maxpos+1);

  do {
        if (charMsg[curpos]==0xc3) {
          if (charMsg[curpos+1]==0xb3) {
            charMsg[curpos]=0xf3;           //ó
            fnd = true;  
          } else if (charMsg[curpos+1]==0x93) {
            charMsg[curpos]=0xd3;           //Ó
            fnd = true;  
          }
        } else if (charMsg[curpos]==0xc4) {
          if (charMsg[curpos+1]==0x85) {
                charMsg[curpos]=0xb1;           //ą
                fnd = true;  
          } else if (charMsg[curpos+1]==0x87) {
                charMsg[curpos]=0xe6;           //ć
                fnd = true;  
          } else if (charMsg[curpos+1]==0x99) {
                charMsg[curpos]=0xea;           //ę
                fnd = true;  
          } else if (charMsg[curpos+1]==0x84) {
                charMsg[curpos]=0xa1;           //Ą
                fnd = true;  
          } else if (charMsg[curpos+1]==0x86) {
                charMsg[curpos]=0xc6;           //Ć
                fnd = true;  
          } else if (charMsg[curpos+1]==0x98) {
                charMsg[curpos]=0xca;           //Ę
                fnd = true;  
          }               
        } else if (charMsg[curpos]==0xc5) {
          if (charMsg[curpos+1]==0x82) {
            charMsg[curpos]=0xb3;           //ł
            fnd = true;  
          } else if (charMsg[curpos+1]==0x84) {
            charMsg[curpos]=0xf1;           //ń
            fnd = true;  
          } else if (charMsg[curpos+1]==0x9b) {
            charMsg[curpos]=0xb6;           //ś
            fnd = true;  
          } else if (charMsg[curpos+1]==0xba) {
            charMsg[curpos]=0xbc;           //ź
            fnd = true;  
          } else if (charMsg[curpos+1]==0xbc) {
            charMsg[curpos]=0xbf;           //ż
            fnd = true;  
          } else if (charMsg[curpos+1]==0x81) {
            charMsg[curpos]=0xa3;           //Ł
            fnd = true;  
          } else if (charMsg[curpos+1]==0x83) {
            charMsg[curpos]=0xd1;           //Ń
            fnd = true;  
          } else if (charMsg[curpos+1]==0x9a) {
            charMsg[curpos]=0xa6;           //Ś
            fnd = true;  
          } else if (charMsg[curpos+1]==0xb9) {
            charMsg[curpos]=0xac;           //Ź
            fnd = true;  
          } else if (charMsg[curpos+1]==0xbb) {
            charMsg[curpos]=0xaf;           //Ż
            fnd = true;  
          }               
        }

    if (fnd) {
      fnd = false;
      memmove(charMsg+curpos+1,charMsg+curpos+2,maxpos-curpos);
      maxpos--; 
    }

    curpos++;

  } while (curpos < maxpos); 

  return String(charMsg);
}
//------------------------------------------------------------------------------
void getArgs(AsyncWebServerRequest *request) {
  if(request->hasArg("t")) {
    String intxt = request->arg("t");
    intxt.trim();
    intxt += " ";
    if (intxt.length() < (MAXTEXTLEN - 2)) {
#if defined ( SERIALDEBUG )
      Serial.println(intxt);
#endif
      outMsg = utf8toiso(intxt);
      matrix.clear();
      matrix.message(outMsg);
    }
  }

  if(request->hasArg("s")) {
    int tmpspeed = atoi(request->arg("s").c_str());
    if ((tmpspeed >= 30) && (tmpspeed <= 150)) {
      matrix.scrollSpeed=tmpspeed;  
    } 
  }
}
//------------------------------------------------------------------------------
void handleCmd(AsyncWebServerRequest *request) {
  getArgs(request);

  request->send(200);
}
//------------------------------------------------------------------------------
void handleRoot(AsyncWebServerRequest *request) {
  getArgs(request);

  String temp = "<!DOCTYPE HTML>";
  temp += "<html>";
  temp += "<head>";
  temp += "<META name='viewport' content='width=device-width, initial-scale=1'>";
  temp += "<title>Text scroller</title>";
  temp += "<style>";
  temp += "body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }";
  temp += "</style>";
  temp += "</head>";
  temp += "<body>";
  temp += "<center>";
  temp += "<h1>Enter text</h1>";
  temp += "<form action='http://";
  temp += WiFi.localIP().toString();
  temp += "' method='POST'>";
  temp += "Text: <input type='text' name='t' value='";
  temp += String(outMsg);
  temp += "' /><br><br>";

  temp += "Speed: <input type='range' name='s' id='s' min='30' max='150' step='5' value='";
  temp += String(matrix.scrollSpeed);
  temp += "' oninput='sval.value=s.value' />";

  temp += "<output name='sval' id='sval' for='s'>";
  temp += String(matrix.scrollSpeed);
  temp += "</output><br><br>";

  temp += "<input type='submit' value='Send' />";
  temp += "</form>";
  temp += "</center>";
  temp += "</body>";
  temp += "</html>";

  request->send(200, "text/html", temp); 
}
//------------------------------------------------------------------------------
void afterGotIp() {
  String myIp = "";
  myIp = WiFi.localIP().toString();
#if defined ( SERIALDEBUG )  
  Serial.println();
  Serial.print("Local IP: ");
  Serial.println(myIp);
#endif
  myIp += " ";
  outMsg = myIp;
  matrix.setIntensity(1);
  matrix.clear();
  matrix.message(outMsg);

  ipshow = true;  
}
//------------------------------------------------------------------------------
void waitForWiFi() {
  int i;

  matrix.icon(1);

  for(i=1;i<=8;i++){
    //matrix.display(i);//intensity up
    matrix.setIntensity(i);//intensity up
    delay(50);
  }
  for(i=8;i>=0;i--){
    //matrix.display(i);//intensity down
    matrix.setIntensity(i);//intensity up
    delay(50);
  }
  matrix.clear();
}
//------------------------------------------------------------------------------
void startota() {
  ArduinoOTA.setHostname(devicename.c_str());

  ArduinoOTA.onStart([]() {
    matrix.icon(2);
    otaintens = 8;
    matrix.setIntensity(otaintens);
    otaup = false;
    otalast = millis() + 10;
  }); 
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (otalast < millis()) {   //update every 15ms
      otalast = millis() + 15;
      if(otaup) {
        if(otaintens<8) {
          otaintens++;
          if (otaintens==8) {
            otaup=false;      
          }              
        }
      } else {
        if(otaintens>1) {
          otaintens--;
          if (otaintens==1) {
            otaup=true;      
          }
        }
      }
      matrix.setIntensity(otaintens);      
    }
  });
  ArduinoOTA.onEnd([]() {
    matrix.icon(3);
    otaintens = 8;
    matrix.setIntensity(otaintens);
    otaup = false;    
  });
  ArduinoOTA.onError([](ota_error_t error) {
    otaintens = 1;
    otaup = false;
    matrix.setIntensity(otaintens); 
  });
  ArduinoOTA.begin();
}
//------------------------------------------------------------------------------
#if defined ( SERIALDEBUG )
//------------------------------------------------------------------------------
inline String format_binary8(uint8_t x) {
  char b[sizeof(x)*8+1] = {0};

  for (size_t z = 0; z < sizeof(x)*8; z++)
    b[sizeof(x)*8-1-z] = ((x>>z) & 0x1) ? '#' : ' ';

  return String(b);
}
//------------------------------------------------------------------------------
void clearTerm() {
  Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command  
}
//------------------------------------------------------------------------------
void homeTerm() {
  Serial.write(27);       // ESC command
  Serial.print("[H");     // cursor to home command  
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
void setup() {
#if defined ( SERIALDEBUG )
  Serial.begin(115200);
  Serial.println();
  Serial.println("START");
#endif

  matrix.begin();
  //matrix.flip = false;
  matrix.scrollSpeed = 55;

  waitForWiFi();

  wifiMulti.addAP(ssid, password);

  if(wifiMulti.run() == WL_CONNECTED) {
    afterGotIp();
  }

  startota();

  server.on("/", HTTP_ANY, handleRoot);
  server.on("/cmd", HTTP_GET, handleCmd);
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
  server.begin();

#if defined ( SERIALDEBUG )
  clearTerm();
#endif
}
//------------------------------------------------------------------------------
void loop() {
  if(wifiMulti.run() != WL_CONNECTED) {
    waitForWiFi();
    ipshow = false;
  } else {
    if(!ipshow) {
      afterGotIp();
    }

    ArduinoOTA.handle();
    int res = matrix.scroll();
#if defined ( SERIALDEBUG )
    if (res!=0) {
      homeTerm();
      for(uint8_t i=0;i<8;i++) {
        Serial.println(format_binary8(matrix.disBuffer[i]));
      }
    }
#endif
  }
#if defined ( ESP8266 )  
  yield();
#endif
}
