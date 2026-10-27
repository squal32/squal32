#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

void touchBegin();
bool touchLoadCalibration();
bool touchHeldAtBoot(TFT_eSPI& tft);
void touchCalibrate(TFT_eSPI& tft);
bool touchRead(int16_t& x, int16_t& y);  // coordonnées écran 320 x 240, false si pas d'appui
