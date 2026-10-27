#include "touch.h"
#include <Preferences.h>

// Tactile XPT2046 sur ses propres broches (vérifié par le test 00-identification).
constexpr int T_CLK = 25, T_MOSI = 32, T_MISO = 39, T_CS = 33;
constexpr int PRESS_MIN = 200;

// Correspondance affine écran = a*brutX + b*brutY + c, mesurée sur la carte par le diagnostic à 9 cibles.
// Valeurs par défaut issues de ce diagnostic ; une recalibration les remplace.
static float CX[3] = {0.003270f, 0.089104f, -28.545f};
static float CY[3] = {0.068867f, 0.001723f, -18.402f};

static uint16_t xpt(uint8_t cmd) {
  digitalWrite(T_CS, LOW);
  for (int i = 7; i >= 0; i--) {
    digitalWrite(T_MOSI, (cmd >> i) & 1);
    digitalWrite(T_CLK, HIGH); delayMicroseconds(2);
    digitalWrite(T_CLK, LOW);  delayMicroseconds(2);
  }
  uint16_t v = 0;
  for (int i = 0; i < 16; i++) {
    // Le XPT2046 change sa sortie sur le front descendant : on lit pendant que l'horloge est haute.
    digitalWrite(T_CLK, HIGH); delayMicroseconds(2);
    v = (v << 1) | digitalRead(T_MISO);
    digitalWrite(T_CLK, LOW);  delayMicroseconds(2);
  }
  digitalWrite(T_CS, HIGH);
  return (v >> 3) & 0x0FFF;
}

static int lastRawX = 0, lastRawY = 0;

static int median5(int* v) {
  for (int i = 1; i < 5; i++) {
    int k = v[i], j = i - 1;
    while (j >= 0 && v[j] > k) { v[j + 1] = v[j]; j--; }
    v[j + 1] = k;
  }
  return v[2];
}

static bool rawRead(int& rx, int& ry) {
  int z = (int)xpt(0xB0) + 4095 - (int)xpt(0xC0);
  if (z < PRESS_MIN) return false;
  xpt(0xD0); xpt(0x90);  // lectures à blanc : laissent l'ampli se stabiliser
  int xs[5], ys[5];
  for (int i = 0; i < 5; i++) { xs[i] = xpt(0xD0); ys[i] = xpt(0x90); }
  z = (int)xpt(0xB0) + 4095 - (int)xpt(0xC0);
  if (z < PRESS_MIN) return false;  // le doigt se lève : mesure douteuse
  rx = median5(xs); ry = median5(ys);
  lastRawX = rx; lastRawY = ry;
  return true;
}

void touchLastRaw(int& rx, int& ry) { rx = lastRawX; ry = lastRawY; }

void touchBegin() {
  pinMode(T_CS, OUTPUT); digitalWrite(T_CS, HIGH);
  pinMode(T_CLK, OUTPUT); pinMode(T_MOSI, OUTPUT); pinMode(T_MISO, INPUT);
}

bool touchLoadCalibration() {
  Preferences p;
  p.begin("touch4", true);
  bool ok = p.getBool("ok", false);
  if (ok) {
    p.getBytes("cx", CX, sizeof CX);
    p.getBytes("cy", CY, sizeof CY);
  }
  p.end();
  return true;  // sans calibration enregistrée, les valeurs mesurées par défaut conviennent
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

static void measure(TFT_eSPI& tft, int sx, int sy, int n, int& rx, int& ry) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  char b[40];
  snprintf(b, sizeof b, "Touche le centre de la cible %d / 9", n);
  tft.drawCentreString(b, 160, sy == 120 ? 160 : 112, 2);
  tft.drawCircle(sx, sy, 10, TFT_CYAN);
  tft.drawFastHLine(sx - 14, sy, 29, TFT_CYAN);
  tft.drawFastVLine(sx, sy - 14, 29, TFT_CYAN);
  waitRelease();
  int x, y;
  while (!rawRead(x, y)) delay(10);
  delay(250);
  long sumx = 0, sumy = 0; int k = 0;
  while (k < 15) {
    if (rawRead(x, y)) { sumx += x; sumy += y; k++; }
    else { k = 0; sumx = sumy = 0; }
    delay(20);
  }
  rx = sumx / k; ry = sumy / k;
  tft.fillCircle(sx, sy, 10, TFT_GREEN);
  waitRelease();
}

static double det3(double m[3][3]) {
  return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
         m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

static bool solve3(double m[3][3], double v[3], float out[3]) {
  double d = det3(m);
  if (fabs(d) < 1e-9) return false;
  for (int c = 0; c < 3; c++) {
    double t[3][3];
    for (int r = 0; r < 3; r++) for (int k = 0; k < 3; k++) t[r][k] = k == c ? v[r] : m[r][k];
    out[c] = det3(t) / d;
  }
  return true;
}

void touchCalibrate(TFT_eSPI& tft) {
  const int tx[9] = {30, 160, 290, 30, 160, 290, 30, 160, 290};
  const int ty[9] = {30, 30, 30, 120, 120, 120, 210, 210, 210};
  bool good = false;
  while (!good) {
    double m[3][3] = {{0}}, vx[3] = {0}, vy[3] = {0};
    int rx[9], ry[9];
    for (int i = 0; i < 9; i++) {
      measure(tft, tx[i], ty[i], i + 1, rx[i], ry[i]);
      double r[3] = {(double)rx[i], (double)ry[i], 1.0};
      for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) m[a][b] += r[a] * r[b];
        vx[a] += r[a] * tx[i];
        vy[a] += r[a] * ty[i];
      }
    }
    float nx[3], ny[3];
    double worst = 1e9;
    if (solve3(m, vx, nx) && solve3(m, vy, ny)) {
      worst = 0;
      for (int i = 0; i < 9; i++) {
        double ex = nx[0] * rx[i] + nx[1] * ry[i] + nx[2] - tx[i];
        double ey = ny[0] * rx[i] + ny[1] * ry[i] + ny[2] - ty[i];
        worst = max(worst, hypot(ex, ey));
      }
    }
    good = worst < 25;  // au-delà, une cible a sans doute été mal touchée
    Serial.printf("Calibration: pire ecart %.1f px -> %s\n", worst, good ? "ok" : "refusee");
    if (good) {
      memcpy(CX, nx, sizeof CX);
      memcpy(CY, ny, sizeof CY);
    } else {
      tft.fillScreen(TFT_BLACK);
      tft.drawCentreString("Une cible a ete mal touchee, on recommence", 160, 110, 2);
      delay(1500);
    }
  }
  Preferences p;
  p.begin("touch4", false);
  p.putBool("ok", true);
  p.putBytes("cx", CX, sizeof CX);
  p.putBytes("cy", CY, sizeof CY);
  p.end();
  tft.fillScreen(TFT_BLACK);
}

bool touchRead(int16_t& x, int16_t& y) {
  int rx, ry;
  if (!rawRead(rx, ry)) return false;
  float px = CX[0] * rx + CX[1] * ry + CX[2];
  float py = CY[0] * rx + CY[1] * ry + CY[2];
  x = constrain((int)lroundf(px), 0, 319);
  y = constrain((int)lroundf(py), 0, 239);
  return true;
}
