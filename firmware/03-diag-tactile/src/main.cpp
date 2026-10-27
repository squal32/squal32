// Diagnostic du tactile XPT2046.
// 1) Pression en direct  2) 9 cibles à toucher  3) calcul de la correspondance et de l'erreur  4) dessin libre.
#include <Arduino.h>
#include <TFT_eSPI.h>

constexpr int TFT_BL_PIN = 21;
constexpr int T_CLK = 25, T_MOSI = 32, T_MISO = 39, T_CS = 33;
constexpr int PRESS_MIN = 250;

TFT_eSPI tft;

uint16_t xpt(uint8_t cmd) {
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

int pressure() { return (int)xpt(0xB0) + 4095 - (int)xpt(0xC0); }

int median5(int* v) {
  for (int i = 1; i < 5; i++) {
    int k = v[i], j = i - 1;
    while (j >= 0 && v[j] > k) { v[j + 1] = v[j]; j--; }
    v[j + 1] = k;
  }
  return v[2];
}

bool rawRead(int& rx, int& ry, int& z) {
  z = pressure();
  if (z < PRESS_MIN) return false;
  xpt(0xD0); xpt(0x90);
  int xs[5], ys[5];
  for (int i = 0; i < 5; i++) { xs[i] = xpt(0xD0); ys[i] = xpt(0x90); }
  if (pressure() < PRESS_MIN) return false;
  rx = median5(xs); ry = median5(ys);
  return true;
}

void waitRelease() {
  int x, y, z;
  uint32_t last = millis();
  while (millis() - last < 300) { if (rawRead(x, y, z)) last = millis(); delay(10); }
}

// ---- étape 1 : pression en direct ----
void pressureTest() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("1/3  Test de pression (15 s)", 160, 6, 2);
  tft.drawCentreString("Appuie doucement, puis fort, a plusieurs endroits", 160, 26, 2);
  int zmin = 99999, zmax = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < 15000) {
    int z = pressure();
    bool down = z >= PRESS_MIN;
    if (z > 50) { zmin = min(zmin, z); zmax = max(zmax, z); }
    int w = constrain(z * 280 / 5000, 0, 280);
    tft.fillRect(20, 80, w, 30, down ? TFT_GREEN : TFT_ORANGE);
    tft.fillRect(20 + w, 80, 280 - w, 30, TFT_DARKGREY);
    int th = PRESS_MIN * 280 / 5000;
    tft.drawFastVLine(20 + th, 74, 42, TFT_RED);
    char b[64];
    snprintf(b, sizeof b, "  Z = %4d   %s   ", z, down ? "APPUI" : "rien ");
    tft.drawCentreString(b, 160, 130, 4);
    snprintf(b, sizeof b, "  min %d  max %d   (seuil %d)  ", zmin == 99999 ? 0 : zmin, zmax, PRESS_MIN);
    tft.drawCentreString(b, 160, 170, 2);
    snprintf(b, sizeof b, "  reste %lu s  ", (unsigned long)(15 - (millis() - t0) / 1000));
    tft.drawCentreString(b, 160, 200, 2);
    delay(40);
  }
  Serial.printf("PRESSION min=%d max=%d seuil=%d\n", zmin == 99999 ? 0 : zmin, zmax, PRESS_MIN);
}

// ---- étape 2 : 9 cibles ----
const int TGT_X[9] = {30, 160, 290, 30, 160, 290, 30, 160, 290};
const int TGT_Y[9] = {30, 30, 30, 120, 120, 120, 210, 210, 210};
int RAW_X[9], RAW_Y[9], RAW_Z[9];

void target(int i) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  char b[48];
  snprintf(b, sizeof b, "2/3  Touche le centre de la cible %d / 9", i + 1);
  tft.drawCentreString(b, 160, TGT_Y[i] == 120 ? 160 : 112, 2);
  tft.drawCircle(TGT_X[i], TGT_Y[i], 10, TFT_CYAN);
  tft.drawFastHLine(TGT_X[i] - 14, TGT_Y[i], 29, TFT_CYAN);
  tft.drawFastVLine(TGT_X[i], TGT_Y[i] - 14, 29, TFT_CYAN);
  waitRelease();
  int x, y, z;
  while (!rawRead(x, y, z)) delay(10);
  delay(250);
  long sx = 0, sy = 0, sz = 0; int n = 0;
  while (n < 15) {
    if (rawRead(x, y, z)) { sx += x; sy += y; sz += z; n++; }
    else { n = 0; sx = sy = sz = 0; }
    delay(20);
  }
  RAW_X[i] = sx / n; RAW_Y[i] = sy / n; RAW_Z[i] = sz / n;
  Serial.printf("CIBLE %d ecran=(%d,%d) brut=(%d,%d) z=%d\n", i + 1, TGT_X[i], TGT_Y[i], RAW_X[i], RAW_Y[i], RAW_Z[i]);
  tft.fillCircle(TGT_X[i], TGT_Y[i], 10, TFT_GREEN);
  waitRelease();
}

// ---- étape 3 : correspondance affine écran = a*bx + b*by + c (gère l'échange et l'inversion des axes) ----
double CX[3], CY[3];

bool solve3(double m[3][3], double v[3], double out[3]) {
  double det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
               m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
  if (fabs(det) < 1e-9) return false;
  for (int c = 0; c < 3; c++) {
    double t[3][3];
    for (int r = 0; r < 3; r++) for (int k = 0; k < 3; k++) t[r][k] = k == c ? v[r] : m[r][k];
    out[c] = (t[0][0] * (t[1][1] * t[2][2] - t[1][2] * t[2][1]) - t[0][1] * (t[1][0] * t[2][2] - t[1][2] * t[2][0]) +
              t[0][2] * (t[1][0] * t[2][1] - t[1][1] * t[2][0])) / det;
  }
  return true;
}

bool fit() {
  double m[3][3] = {{0}}, vx[3] = {0}, vy[3] = {0};
  for (int i = 0; i < 9; i++) {
    double r[3] = {(double)RAW_X[i], (double)RAW_Y[i], 1.0};
    for (int a = 0; a < 3; a++) {
      for (int b = 0; b < 3; b++) m[a][b] += r[a] * r[b];
      vx[a] += r[a] * TGT_X[i];
      vy[a] += r[a] * TGT_Y[i];
    }
  }
  return solve3(m, vx, CX) && solve3(m, vy, CY);
}

void mapRaw(int rx, int ry, int& x, int& y) {
  x = (int)lround(CX[0] * rx + CX[1] * ry + CX[2]);
  y = (int)lround(CY[0] * rx + CY[1] * ry + CY[2]);
}

void report() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("Resultat : ecart de chaque cible (pixels)", 160, 4, 2);
  double worst = 0, sum = 0;
  for (int i = 0; i < 9; i++) {
    int x, y;
    mapRaw(RAW_X[i], RAW_Y[i], x, y);
    double e = hypot(x - TGT_X[i], y - TGT_Y[i]);
    worst = max(worst, e); sum += e;
    tft.drawCircle(TGT_X[i], TGT_Y[i], 10, TFT_CYAN);
    tft.fillCircle(x, y, 4, e < 8 ? TFT_GREEN : e < 15 ? TFT_YELLOW : TFT_RED);
    char b[12];
    snprintf(b, sizeof b, "%.0f", e);
    tft.drawCentreString(b, TGT_X[i], TGT_Y[i] + (TGT_Y[i] > 200 ? -30 : 14), 2);
  }
  char b[64];
  snprintf(b, sizeof b, "moyenne %.1f px   pire %.1f px", sum / 9, worst);
  tft.drawCentreString(b, 160, 72, 2);
  Serial.printf("ECART moyenne=%.1f pire=%.1f\n", sum / 9, worst);
  Serial.printf("COEF X %.6f %.6f %.3f\n", CX[0], CX[1], CX[2]);
  Serial.printf("COEF Y %.6f %.6f %.3f\n", CY[0], CY[1], CY[2]);
  delay(8000);
}

// ---- étape 4 : dessin libre ----
void freeDraw() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("3/3  Dessine : le point doit suivre ton doigt", 160, 4, 2);
  for (int i = 0; i < 9; i++) tft.drawCircle(TGT_X[i], TGT_Y[i], 6, TFT_DARKGREY);
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_BL_PIN, OUTPUT); digitalWrite(TFT_BL_PIN, HIGH);
  pinMode(T_CS, OUTPUT); digitalWrite(T_CS, HIGH);
  pinMode(T_CLK, OUTPUT); pinMode(T_MOSI, OUTPUT); pinMode(T_MISO, INPUT);
  tft.init();
  tft.setRotation(1);
  pressureTest();
  do {
    for (int i = 0; i < 9; i++) target(i);
  } while (!fit());
  report();
  freeDraw();
}

void loop() {
  int rx, ry, z, x, y;
  if (rawRead(rx, ry, z)) {
    mapRaw(rx, ry, x, y);
    tft.fillCircle(x, y, 3, TFT_GREEN);
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 150) {
      lastPrint = millis();
      char b[48];
      snprintf(b, sizeof b, " x=%3d y=%3d z=%4d ", x, y, z);
      tft.drawString(b, 4, 224, 2);
    }
  }
  delay(5);
}
