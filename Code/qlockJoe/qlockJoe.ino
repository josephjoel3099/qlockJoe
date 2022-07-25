#define FASTLED_FORCE_SOFTWARE_SPI
#define FASTLED_FORCE_SOFTWARE_PINS

#include <Wire.h>
#include <ds3231.h>
#include <FastLED.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid     = "Annmary Villa";
const char *password = "74740780";

const long utcOffsetInSeconds = 19800;

WiFiUDP ntpUDP;
NTPClient t(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds);

CRGB leds[330];

boolean hasRun=false;
struct ts tds; 
int minutes;
int hours;
int CurrentTime[4]={99,99,99,99};
int PreviousTime[4]={99,99,99,99};
int n=0;
int on_leds[28],off_leds[28];

int Time_Comp[21][7]={ {196,193,190,999,999,999,999},  //ONE
                       {223,226,229,999,999,999,999},  //TWO
                       {178,175,172,169,166,999,999},  //THREE
                       {199,202,205,208,999,999,999},  //FOUR
                       {211,214,217,220,999,999,999},  //FIVE
                       {187,184,181,999,999,999,999},  //SIX
                       {265,268,271,274,277,999,999},  //SEVEN
                       {262,259,256,253,250,999,999},  //EIGHT
                       {154,157,160,163,999,999,999},  //NINE
                       {328,325,322,999,999,999,999},  //TEN
                       {247,244,241,238,235,232,999},  //ELEVEN
                       {280,283,286,289,292,295,999},  //TWELVE
                       {133,136,139,142,999,999,999},  //PAST
                       {103,100,999,999,999,999,999},  //TO           
                       { 85, 88, 91, 94,999,999,999},  //FIVE
                       {115,112,109,999,999,999,999},  //TEN
                       { 58, 55, 52, 49, 46, 43, 40},  //QUARTER
                       { 67, 70, 73, 76, 79, 82,999},  //TWENTY
                       {130,127,124,121,999,999,999},  //HALF
                       {  1,  4,999, 10, 13,999,999},  //IT IS
                       {313,310,307,304,301,298,999} };//O'CLOCK

int Wifi_Comp[4][7]={ {214,999,999,999,999,999,999},
                      {184,181,178,999,999,999,999},
                      {142,145,148,151,154,999,999},
                      {124,121,118,115,112,109,106} };


void LitAll(){
  for(int i=0;i<=100;i++){
  LEDS.setBrightness(i);  
  FastLED.show();
  delay(1);
 }
 hasRun=true;
}

void setup() {  
  FastLED.addLeds<WS2812B, 16, GRB>(leds,   0, 66).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2812B,  0, GRB>(leds,  66, 66).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2812B,  2, GRB>(leds, 132, 66).setCorrection( TypicalLEDStrip ); 
  FastLED.addLeds<WS2812B, 14, GRB>(leds, 198, 66).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2812B, 12, GRB>(leds, 264, 66).setCorrection( TypicalLEDStrip );
  FastLED.clear();
  FastLED.show();
  LEDS.setBrightness(100);

  for(int i=0;i<28;i++){
    on_leds[i]=off_leds[i]=999;
  }

  delay(100);

  Wire.begin();
  DS3231_init(DS3231_CONTROL_ADDR);

  pinMode(LED_BUILTIN,OUTPUT);
  WiFi.begin(ssid, password);
  Serial.begin(9600);
  while ( WiFi.status() != WL_CONNECTED && n<10) {
    for(int i=0;i<4;i++){
      for(int j=0;j<7;j++){
        leds[Wifi_Comp[i][j]-1]=leds[Wifi_Comp[i][j]]=leds[Wifi_Comp[i][j]+1]=CRGB::White;
      }
      LEDS.setBrightness(100);
      FastLED.show();
      delay(300);
    }
    FastLED.clear();
    FastLED.show();
    n++;
  }

  //Connected
  if(WiFi.status() == WL_CONNECTED){    
    t.begin();
    t.update();
     
    tds.hour=t.getHours(); 
    tds.min=t.getMinutes();
    tds.sec=t.getSeconds();
    DS3231_set(tds);
    
    for(int c=0;c<3;c++){
      digitalWrite(LED_BUILTIN,HIGH);
      delay (100);
      digitalWrite(LED_BUILTIN,LOW);
      delay(100);
    }
  }
}

void loop(){
  DS3231_get(&tds);

  PreviousTime[0]=CurrentTime[0];
  PreviousTime[1]=CurrentTime[1];
  PreviousTime[2]=CurrentTime[2];
  PreviousTime[3]=CurrentTime[3];

  minutes=int((tds.min)/5)*5;
  if (minutes<35 and minutes>=0) hours=tds.hour ;else hours=tds.hour+1 ;
  if (minutes==0)                      {CurrentTime[0]=20;CurrentTime[1]=99;}
  if ((minutes==5)  or (minutes==55))  {CurrentTime[0]=14;CurrentTime[1]=99;}
  if ((minutes==10) or (minutes==50))  {CurrentTime[0]=15;CurrentTime[1]=99;}
  if ((minutes==15) or (minutes==45))  {CurrentTime[0]=16;CurrentTime[1]=99;}
  if ((minutes==20) or (minutes==40))  {CurrentTime[0]=17;CurrentTime[1]=99;}
  if ((minutes==25) or (minutes==35))  {CurrentTime[0]=17;CurrentTime[1]=14;}
  if (minutes==30)                     {CurrentTime[0]=18;CurrentTime[1]=99;}  
  if (minutes>= 5 and minutes<35)  CurrentTime[2]=12; 
  if (minutes>=35 and minutes<=59) CurrentTime[2]=13;
  if (minutes==0)                  CurrentTime[2]=99;
  CurrentTime[3]=hours - int(hours/12)*12-1; 
  if (CurrentTime[3]<0) CurrentTime[3]=CurrentTime[3]+12;
  
  for (int j=0;j<7;j++){
    leds[Time_Comp[19][j]-1]=leds[Time_Comp[19][j]]=leds[Time_Comp[19][j]+1]=CRGB::White;
  }

  if (PreviousTime[0]!=CurrentTime[0] or PreviousTime[1]!=CurrentTime[1]){

    for(int i=0,j=0,k=0;i<28;i++,j=int(i/7)){
      if(CurrentTime[j]!=99) on_leds[i]=Time_Comp[CurrentTime[j]][k];
      else on_leds[i]=999;
      k++;
      if(k>6) k=0;
    }

    for(int i=0,j=0,k=0;i<28;i++,j=int(i/7)){
      if(PreviousTime[j]!=99 && Time_Comp[PreviousTime[j]][k]!=on_leds[i]){
        off_leds[i]=Time_Comp[PreviousTime[j]][k];
      }
      else off_leds[i]=999;
      k++;
      if(k>6) k=0;
    }

    for(int i=0;i<28;i++){
      leds[on_leds[i]-1]=leds[on_leds[i]]=leds[on_leds[i]+1]=CRGB::White;
    }
  }

  for(int i=0;i<28;i++){
    leds[off_leds[i]-1].fadeToBlackBy(20);
    leds[off_leds[i]].fadeToBlackBy(20);
    leds[off_leds[i]+1].fadeToBlackBy(20);
  }

  if(hasRun==false) LitAll();
  else FastLED.show();
}
