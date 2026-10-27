#include "touch.h"
#include <Preferences.h>

// Tactile XPT2046 sur ses propres broches (vérifié par le test 00-identification).
constexpr int T_CLK = 25, T_MOSI = 32, T_MISO = 39, T_CS = 33;
constexpr int PRESS_MIN = 400;
constexpr int CAL_A_X = 20, CAL_A_Y = 20, CAL_B_X = 300, CAL_B_Y = 220;

struct Cal { bool swap; int ax0, ax1, ay0, ay1; bool ok; } cal = {true, 0, 0, 0, 0, false};

static uint16_t xpt(uint8_t cmd) {
  digitalWrite(T_CS, LOW);
  for (int i = 7; i >= 0; i--) {
    digitalWrite(T_MOSI, (cmd >> i) & 1);
    digitalWrite(T_CLK, HIGH); delayMicroseconds(2);
    digitalWrite(T_CLK, LOW);  delayMicroseconds(2);
  }
  uint16_t v = 0;
  for (int i = 0; i < 16; i++) {
    digitalWrite(T_CLK, HIGH); delayMicroseconds(2);
    digitalWrite(T_CLK, LOW);  delayMicroseconds(2);
    v = (v << 1) | digitalRead(T_MISO);
  }
  digitalWrite(T_CS, HIGH);
  return (v >> 3) & 0x0FFF;
}

static bool rawRead(int& rx, int& ry) {
  int z = (int)xpt(0xB0) + 4095 - (int)xpt(0xC0);
  if (z < PRESS_MIN) return false;
  long sx = 0, sy = 0;
  for (int i = 0; i < 3; i++) { sx += xpt(0xD0); sy += xpt(0x90); }
  rx = sx / 3; ry = sy / 3;
  return true;
}

void touchBegin() {
  pinMode(T_CS, OUTPUT); digitalWrite(T_CS, HIGH);
  pinMode(T_CLK, OUTPUT); pinMode(T_MOSI, OUTPUT); pinMode(T_MISO, INPUT);
}

bool touchLoadCalibration() {
  Preferences p;
  p.begin("touch3", true);
  cal.ok = p.getBool("ok", false);
  cal.swap = p.getBool("swap", true);
  cal.ax0 = p.getInt("ax0", 0); cal.ax1 = p.getInt("ax1", 0);
  cal.ay0 = p.getInt("ay0", 0); cal.ay1 = p.getInt("ay1", 0);
  p.end();
  return cal.ok && cal.ax0 != cal.ax1 && cal.ay0 != cal.ay1;
}

bool touchHeldAtBoot(TFT_eSPI& tft) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("Maintiens l'ecran pour recalibrer", 160, 110, 2);
  uint32_t t0 = millis(), held = 0;
  int rx, ry;
  while (millis() - t0 < 1500) {
    if (rawRead(rx, ry)) { if (!held) held = millis(); if (millis() - held > 400) return true; }
    else held = 0;
    delay(20);
  }
  return false;
}

static void waitRelease() {
  int x, y;
  uint32_t lastPress = millis();
  while (millis() - lastPress < 300) {
    if (rawRead(x, y)) lastPress = millis();
    delay(10);
  }
}

static void waitPoint(TFT_eSPI& tft, int sx, int sy, const char* txt, int& rx, int& ry) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(txt, 160, 110, 2);
  tft.drawFastHLine(sx - 10, sy, 21, TFT_CYAN);
  tft.drawFastVLine(sx, sy - 10, 21, TFT_CYAN);
  tft.drawCircle(sx, sy, 6, TFT_CYAN);
  waitRelease();  // le doigt doit avoir quitté la croix précédente
  int x, y;
  while (!rawRead(x, y)) delay(10);
  delay(250);     // laisser le doigt se poser avant de mesurer
  long sumx = 0, sumy = 0; int n = 0;
  while (n < 12) {
    if (rawRead(x, y)) { sumx += x; sumy += y; n++; }
    else { n = 0; sumx = sumy = 0; }  // doigt levé : on recommence
    delay(20);
  }
  rx = sumx / n; ry = sumy / n;
  waitRelease();
}

void touchCalibrate(TFT_eSPI& tft) {
  // Quatre croix aux coins : l'axe brut de l'abscisse et celui de l'ordonnée sont déduits des mesures.
  const int px[4] = {CAL_A_X, CAL_B_X, CAL_A_X, CAL_B_X};
  const int py[4] = {CAL_A_Y, CAL_A_Y, CAL_B_Y, CAL_B_Y};
  const char* msg[4] = {"Croix en haut a gauche", "Croix en haut a droite", "Croix en bas a gauche", "Croix en bas a droite"};
  bool good = false;
  while (!good) {
    int rx[4], ry[4];
    for (int i = 0; i < 4; i++) waitPoint(tft, px[i], py[i], msg[i], rx[i], ry[i]);
    // L'abscisse écran est portée par l'axe brut qui varie le plus le long d'une même ligne horizontale.
    int dX = abs(rx[1] - rx[0]) + abs(rx[3] - rx[2]);
    int dY = abs(ry[1] - ry[0]) + abs(ry[3] - ry[2]);
    bool xFromY = dY > dX;
    const int* sx = xFromY ? ry : rx;
    const int* sy = xFromY ? rx : ry;
    cal.swap = xFromY;
    cal.ax0 = (sx[0] + sx[2]) / 2; cal.ax1 = (sx[1] + sx[3]) / 2;
    cal.ay0 = (sy[0] + sy[1]) / 2; cal.ay1 = (sy[2] + sy[3]) / 2;
    good = abs(cal.ax1 - cal.ax0) > 800 && abs(cal.ay1 - cal.ay0) > 800;
    Serial.printf("Calibration: swap=%d ax=%d..%d ay=%d..%d -> %s\n", cal.swap, cal.ax0, cal.ax1, cal.ay0, cal.ay1, good ? "ok" : "refusee");
    if (!good) {
      tft.fillScreen(TFT_BLACK);
      tft.drawCentreString("Mesure incoherente, on recommence", 160, 110, 2);
      delay(1500);
    }
  }
  cal.ok = true;
  Preferences p;
  p.begin("touch3", false);
  p.putBool("ok", true); p.putBool("swap", cal.swap);
  p.putInt("ax0", cal.ax0); p.putInt("ax1", cal.ax1);
  p.putInt("ay0", cal.ay0); p.putInt("ay1", cal.ay1);
  p.end();

  // Test : un point vert suit le doigt pendant 6 secondes
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("Test : dessine avec le doigt", 160, 100, 2);
  for (int i = 0; i < 4; i++) {
    tft.drawFastHLine(px[i] - 10, py[i], 21, TFT_DARKGREY);
    tft.drawFastVLine(px[i], py[i] - 10, 21, TFT_DARKGREY);
  }
  uint32_t t0 = millis();
  int16_t x, y;
  while (millis() - t0 < 6000) {
    if (touchRead(x, y)) tft.fillCircle(x, y, 3, TFT_GREEN);
    delay(5);
  }
  tft.fillScreen(TFT_BLACK);
}

bool touchRead(int16_t& x, int16_t& y) {
  int rx, ry;
  if (!rawRead(rx, ry)) return false;
  int srcX = cal.swap ? ry : rx, srcY = cal.swap ? rx : ry;
  long px = CAL_A_X + (long)(srcX - cal.ax0) * (CAL_B_X - CAL_A_X) / (cal.ax1 - cal.ax0);
  long py = CAL_A_Y + (long)(srcY - cal.ay0) * (CAL_B_Y - CAL_A_Y) / (cal.ay1 - cal.ay0);
  x = constrain(px, 0, 319);
  y = constrain(py, 0, 239);
  return true;
}
