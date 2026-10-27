#pragma once
#include <Arduino.h>

// Wi-Fi : réseaux enregistrés depuis l'écran (mémoire de la carte) + secrets.h facultatif.
void wifiInit();
void wifiLoop();
bool wifiConnected();
String wifiSsid();

// Recherche des réseaux (suspend les essais automatiques jusqu'à wifiResume)
void wifiScanStart();
int wifiScanCount();  // -1 : en cours, 0 ou plus : nombre de réseaux trouvés
const char* wifiScanSsid(int i);
int wifiScanRssi(int i);
bool wifiScanOpen(int i);
void wifiResume();

void wifiConnect(const char* ssid, const char* pass);  // enregistre puis se connecte
void wifiForgetAll();
