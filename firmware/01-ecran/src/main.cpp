// Test d'affichage (étape 3). Dessine des barres de couleur et des repères.
// Toucher l'écran : bascule l'inversion des couleurs (état affiché et écrit sur le port série).
#include <Arduino.h>
#include <TFT_eSPI.h>

constexpr int TFT_BL_PIN = 21;
constexpr int T_CLK = 25, T_MOSI = 32, T_MISO = 39, T_CS = 33;

TFT_eSPI tft;
bool inverted = false;

uint16_t xptRead(uint8_t cmd) {
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

void drawTest() {
  tft.fillScreen(TFT_BLACK);
  const uint16_t cols[5] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE, TFT_BLACK};
  const char* names[5] = {"ROUGE", "VERT", "BLEU", "BLANC", "NOIR"};
  for (int i = 0; i < 5; i++) {
    tft.fillRect(i * 64, 40, 64, 120, cols[i]);
    tft.setTextColor(i == 3 ? TFT_BLACK : TFT_WHITE, cols[i]);
    tft.drawCentreString(names[i], i * 64 + 32, 92, 2);
  }
  tft.drawRect(0, 0, 320, 240, TFT_YELLOW);          // bord de l'écran
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("1", 6, 6, 4);
  tft.drawRightString("2", 314, 6, 4);
  tft.drawString("3", 6, 212, 4);
  tft.drawRightString("4", 314, 212, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(inverted ? "Inversion : OUI" : "Inversion : NON", 160, 8, 2);
  tft.drawCentreString("Touche l'ecran pour basculer", 160, 170, 2);
  for (int x = 0; x < 320; x++) {                      // dégradé de gris
    uint8_t g = x * 255 / 319;
    tft.drawFastVLine(x, 195, 12, tft.color565(g, g, g));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(T_CS, OUTPUT); digitalWrite(T_CS, HIGH);
  pinMode(T_CLK, OUTPUT); pinMode(T_MOSI, OUTPUT); pinMode(T_MISO, INPUT);
  pinMode(TFT_BL_PIN, OUTPUT); digitalWrite(TFT_BL_PIN, HIGH);
  tft.init();
  tft.setRotation(1);  // paysage 320 x 240
  tft.invertDisplay(inverted);
  drawTest();
  Serial.println("Test affichage pret. Variante: " 
#if defined(ST7789_DRIVER)
    "ST7789"
#else
    "ILI9341_2"
#endif
  );
}

uint32_t lastToggle = 0;
void loop() {
  int z = (int)xptRead(0xB0) + 4095 - (int)xptRead(0xC0);
  if (z > 400 && millis() - lastToggle > 700) {
    lastToggle = millis();
    inverted = !inverted;
    tft.invertDisplay(inverted);
    drawTest();
    Serial.printf("Touche X=%u Y=%u Z=%d -> inversion=%d\n", xptRead(0xD0), xptRead(0x90), z, inverted);
  }
  delay(50);
}
