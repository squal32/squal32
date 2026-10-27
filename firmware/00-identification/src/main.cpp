// Identification de la carte ESP32-2432S028 : sortie série uniquement.
// Les broches ci-dessous sont celles couramment citées pour cette carte ;
// ce programme sert justement à vérifier qu'elles correspondent à TA carte.
#include <Arduino.h>
#include <SPI.h>

constexpr int TFT_SCLK = 14, TFT_MOSI = 13, TFT_MISO = 12, TFT_CS = 15, TFT_DC = 2, TFT_BL = 21;
constexpr int T_CLK = 25, T_MOSI = 32, T_MISO = 39, T_CS = 33, T_IRQ = 36;
constexpr int LED_R = 4, LED_G = 16, LED_B = 17, LDR = 34;

SPIClass tftSpi(HSPI);

uint32_t readReg(uint8_t cmd, uint8_t n) {
  tftSpi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_DC, LOW);
  tftSpi.transfer(cmd);
  digitalWrite(TFT_DC, HIGH);
  tftSpi.transfer(0);  // octet factice
  uint32_t v = 0;
  for (uint8_t i = 0; i < n; i++) v = (v << 8) | tftSpi.transfer(0);
  digitalWrite(TFT_CS, HIGH);
  tftSpi.endTransaction();
  return v;
}

void writeCmd(uint8_t cmd) {
  tftSpi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_DC, LOW);
  tftSpi.transfer(cmd);
  digitalWrite(TFT_CS, HIGH);
  tftSpi.endTransaction();
}

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

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n===== IDENTIFICATION CARTE =====");
  Serial.printf("Puce: %s rev %d, %d coeur(s), %d MHz\n", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores(), ESP.getCpuFreqMHz());
  Serial.printf("Flash: %u Mo, PSRAM: %u octets\n", ESP.getFlashChipSize() / (1024 * 1024), ESP.getPsramSize());

  pinMode(TFT_BL, OUTPUT);
  pinMode(TFT_CS, OUTPUT); digitalWrite(TFT_CS, HIGH);
  pinMode(TFT_DC, OUTPUT);
  pinMode(T_CS, OUTPUT); digitalWrite(T_CS, HIGH);
  pinMode(T_CLK, OUTPUT); pinMode(T_MOSI, OUTPUT);
  pinMode(T_MISO, INPUT); pinMode(T_IRQ, INPUT);
  pinMode(LED_R, OUTPUT); pinMode(LED_G, OUTPUT); pinMode(LED_B, OUTPUT);
  pinMode(LDR, INPUT);

  // Rétroéclairage : 3 clignotements (si rien ne bouge, la broche 21 est peut-être différente)
  Serial.println("Retroeclairage: 3 clignotements...");
  for (int i = 0; i < 3; i++) { digitalWrite(TFT_BL, HIGH); delay(400); digitalWrite(TFT_BL, LOW); delay(400); }
  digitalWrite(TFT_BL, HIGH);

  // LED RGB (allumée à l'état BAS sur cette carte, à confirmer)
  Serial.println("LED RGB: rouge, vert, bleu...");
  const int leds[3] = {LED_R, LED_G, LED_B};
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_R, HIGH); digitalWrite(LED_G, HIGH); digitalWrite(LED_B, HIGH);
    digitalWrite(leds[i], LOW); delay(600);
  }
  digitalWrite(LED_R, HIGH); digitalWrite(LED_G, HIGH); digitalWrite(LED_B, HIGH);

  // Identifiants du contrôleur d'écran
  tftSpi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  writeCmd(0x01); delay(150);  // reset logiciel
  uint32_t id04 = readReg(0x04, 3), idD3 = readReg(0xD3, 3), st09 = readReg(0x09, 4);
  Serial.printf("RDDID  (0x04): 0x%06X\n", id04);
  Serial.printf("ID4    (0xD3): 0x%06X\n", idD3);
  Serial.printf("Statut (0x09): 0x%08X\n", st09);
  if (idD3 == 0x009341) Serial.println("=> Compatible ILI9341");
  else if (id04 == 0x858552) Serial.println("=> Compatible ST7789");
  else Serial.println("=> Controleur non reconnu ou MISO illisible : envoie-moi ces valeurs.");

  Serial.println("\nTOUCHE L'ECRAN (coins puis centre) : valeurs brutes X, Y, pression.");
}

uint32_t tLdr = 0;
void loop() {
  uint16_t z1 = xptRead(0xB0), z2 = xptRead(0xC0);
  int z = z1 + 4095 - z2;
  if (z > 400) {
    Serial.printf("Touche  X=%4u  Y=%4u  Z=%d  IRQ=%d\n", xptRead(0xD0), xptRead(0x90), z, digitalRead(T_IRQ));
  }
  if (millis() - tLdr > 3000) {
    tLdr = millis();
    Serial.printf("Luminosite ambiante (LDR, GPIO34): %d / 4095\n", analogRead(LDR));
  }
  delay(100);
}
