#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/* ================= CONFIG ================= */

#define NUM_LEDS     440

#define PIN          12

#define WORD_PIXELS  4
#define INVALID_LED  999

#define FADE_STEPS   20
#define FADE_DELAY   30

const char* ssid     = "Annmary Villa";
const char* password = "74740780";

const long utcOffsetInSeconds = 19800;

/* ================= WORD MAP ================= */

int Time_Comp[21][7] = {
    {176, 180, 184, 999, 999, 999, 999}, // ONE     0
    {140, 136, 132, 999, 999, 999, 999}, // TWO     1
    {200, 204, 208, 212, 216, 999, 999}, // THREE   2
    {172, 168, 164, 160, 999, 999, 999}, // FOUR    3
    {156, 152, 148, 144, 999, 999, 999}, // FIVE    4
    {188, 192, 196, 999, 999, 999, 999}, // SIX     5
    { 84,  80,  76,  72,  68, 999, 999}, // SEVEN   6
    { 88,  92,  96, 100, 104, 999, 999}, // EIGHT   7
    {232, 228, 224, 220, 999, 999, 999}, // NINE    8
    {  0,   4,   8, 999, 999, 999, 999}, // TEN     9
    {108, 112, 116, 120, 124, 128, 999}, // ELEVEN  10
    { 64,  60,  56,  52,  48,  44, 999}, // TWELVE  11
    {260, 256, 252, 248, 999, 999, 999}, // PAST    12
    {300, 304, 999, 999, 999, 999, 999}, // TO      13
    {324, 320, 316, 312, 999, 999, 999}, // FIVE    14
    {284, 288, 292, 999, 999, 999, 999}, // TEN     15
    {360, 364, 368, 372, 376, 380, 384}, // QUARTER 16 
    {348, 344, 340, 336, 332, 328, 999}, // TWENTY  17
    {264, 268, 272, 276, 999, 999, 999}, // HALF    18
    {436, 432, 999, 424, 420, 999, 999}, // IT IS   19
    { 20,  24,  28,  32,  36,  40, 999}  // O'CLOCK 20
};

/* ================= LED MATRIX ================= */

class LedMatrix {
  Adafruit_NeoPixel strip;

public:
  LedMatrix() : strip(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800) {}

  void begin() {
    strip.begin();
    strip.setBrightness(128);
    strip.clear();
    strip.show();
  }

  void show() { strip.show(); }

  void setWord(int led, uint8_t v) {
    if (led == INVALID_LED) return;

    for (int i = 0; i < WORD_PIXELS; i++)
      strip.setPixelColor(led + i, strip.Color(v, v, v));
  }

  void fullOn(int led) { setWord(led, 255); }
  void off(int led)    { setWord(led, 0);   }
};

/* ================= WORD CLOCK ================= */

class WordClock {
  int current[4]  = {99,99,99,99};
  int previous[4] = {99,99,99,99};

  int fadeList[28];
  int fadeStep = 0;
  bool fading = false;

public:
  void update(NTPClient &time) {
    memcpy(previous, current, sizeof(current));

    int minutes = (time.getMinutes() / 5) * 5;
    int hours   = time.getHours();

    if (minutes >= 35)
      hours++;

    current[0] = current[1] = 99;

    if (minutes == 5 || minutes == 55)  current[0] = 14;
    if (minutes == 10 || minutes == 50) current[0] = 15;
    if (minutes == 15 || minutes == 45) current[0] = 16;
    if (minutes == 20 || minutes == 40) current[0] = 17;

    if (minutes == 25 || minutes == 35) {
      current[0] = 17;
      current[1] = 14;
    }

    if (minutes == 30)
      current[0] = 18;

    if (minutes == 0)
      current[2] = 20;      // O'CLOCK
    else if (minutes < 35)
      current[2] = 12;      // PAST
    else
      current[2] = 13;      // TO

    current[3] = (hours % 12) - 1;

    if (current[3] < 0)
      current[3] += 12;
  }

  void render(LedMatrix &leds) {

    fadeStep = 0;
    fading = false;

    // --- IT IS always ON ---
    for (int i = 0; i < 7; i++) {
      if (Time_Comp[19][i] != INVALID_LED)
        leds.fullOn(Time_Comp[19][i]);
    }

    for (int i = 0, j = 0, k = 0; i < 28; i++, j = i / 7) {

      int now  = (current[j]  != 99) ? Time_Comp[current[j]][k]  : INVALID_LED;
      int prev = (previous[j] != 99) ? Time_Comp[previous[j]][k] : INVALID_LED;

      if (now != INVALID_LED)
        leds.fullOn(now);

      // ✅ FIXED:
      // O'CLOCK now fades correctly
      // ONLY IT IS is excluded from fade
      if (prev != INVALID_LED &&
          prev != now &&
          prev != 19) {

        fadeList[i] = prev;
        fading = true;

      } else {
        fadeList[i] = INVALID_LED;
      }

      if (++k > 6)
        k = 0;
    }

    // --- O'CLOCK only at exact hour ---
    if (current[2] == 20) {
      for (int i = 0; i < 7; i++) {
        if (Time_Comp[20][i] != INVALID_LED)
          leds.fullOn(Time_Comp[20][i]);
      }
    }

    leds.show();
  }

  void fade(LedMatrix &leds) {

    if (!fading)
      return;

    for (int i = 0; i < 28; i++) {

      if (fadeList[i] != INVALID_LED) {

        uint8_t v = map(fadeStep, 0, FADE_STEPS, 255, 0);
        leds.setWord(fadeList[i], v);
      }
    }

    leds.show();

    fadeStep++;

    if (fadeStep > FADE_STEPS) {

      for (int i = 0; i < 28; i++) {
        if (fadeList[i] != INVALID_LED)
          leds.off(fadeList[i]);
      }

      fading = false;
    }

    delay(FADE_DELAY);
  }
};

/* ================= WIFI ================= */

class WiFiLED {

  int anim[4][7] = {
    {108,999,999,999,999,999,999},
    {156,152,148,999,999,999,999},
    {188,192,196,200,204,999,999},
    {252,248,244,240,236,232,228}
  };

public:

  void connect(LedMatrix &leds) {

    WiFi.begin(ssid, password);

    int i = 0;

    while (WiFi.status() != WL_CONNECTED) {

      if (i > 3) {
        leds.begin();
        i = 0;
      }

      for (int j = 0; j < 7; j++) {

        if (anim[i][j] != INVALID_LED)
          leds.fullOn(anim[i][j]);
      }

      leds.show();

      delay(300);

      i++;
    }

    leds.begin(); // clear LEDs after WiFi connects
  }
};

/* ================= MAIN ================= */

WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", utcOffsetInSeconds);

LedMatrix leds;
WordClock clockLogic;
WiFiLED wifiAnim;

void setup() {

  leds.begin();

  wifiAnim.connect(leds);

  delay(500);

  timeClient.begin();
  timeClient.update();
}

void loop() {

  static int lastMinute = -1;

  timeClient.update();

  if (timeClient.getMinutes() != lastMinute) {

    lastMinute = timeClient.getMinutes();

    clockLogic.update(timeClient);
    clockLogic.render(leds);
  }

  clockLogic.fade(leds);
}