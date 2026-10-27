// Assistant de bureau : interface LVGL avec données de démonstration (étape 4).
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <time.h>
#include "touch.h"
#include "ui.h"

#include "wifi_mgr.h"

constexpr const char* TZ_PARIS = "CET-1CEST,M3.5.0,M10.5.0/3";

constexpr int TFT_BL_PIN = 21;

TFT_eSPI tft;

void backlight(uint8_t level) { ledcWrite(0, level); }
static lv_disp_draw_buf_t drawBuf;
static lv_color_t* bufA = nullptr;  // alloué au démarrage : la mémoire statique de l'ESP32 est limitée

static void flushCb(lv_disp_drv_t* drv, const lv_area_t* a, lv_color_t* c) {
  uint32_t w = a->x2 - a->x1 + 1, h = a->y2 - a->y1 + 1;
  tft.startWrite();
  tft.setAddrWindow(a->x1, a->y1, w, h);
  tft.pushColors((uint16_t*)&c->full, w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(drv);
}

static void touchCb(lv_indev_drv_t*, lv_indev_data_t* data) {
  static bool wasDown = false;
  static int misses = 0;
  static int16_t lx = 0, ly = 0;
  int16_t x, y;
  if (touchRead(x, y)) {
    if (!wasDown) {
      int rx, ry;
      touchLastRaw(rx, ry);
      Serial.printf("appui %d,%d (brut %d,%d)\n", x, y, rx, ry);
    }
    wasDown = true;
    misses = 0;
    lx = x; ly = y;
  } else if (wasDown && ++misses >= 4) {  // le relâchement n'est validé qu'après 4 lectures vides
    wasDown = false;
  }
  data->state = wasDown ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
  data->point.x = lx;
  data->point.y = ly;
}

void setup() {
  Serial.begin(115200);
  ledcSetup(0, 5000, 8);
  ledcAttachPin(TFT_BL_PIN, 0);
  backlight(255);
  tft.init();
  tft.setRotation(1);
  touchBegin();
  if (!touchLoadCalibration() || touchHeldAtBoot(tft)) touchCalibrate(tft);

  lv_init();
  bufA = (lv_color_t*)malloc(320 * 30 * sizeof(lv_color_t));
  lv_disp_draw_buf_init(&drawBuf, bufA, nullptr, 320 * 30);
  static lv_disp_drv_t dispDrv;
  lv_disp_drv_init(&dispDrv);
  dispDrv.hor_res = 320;
  dispDrv.ver_res = 240;
  dispDrv.flush_cb = flushCb;
  dispDrv.draw_buf = &drawBuf;
  lv_disp_drv_register(&dispDrv);

  static lv_indev_drv_t indevDrv;
  lv_indev_drv_init(&indevDrv);
  indevDrv.type = LV_INDEV_TYPE_POINTER;
  indevDrv.read_cb = touchCb;
  lv_indev_drv_register(&indevDrv);

  setenv("TZ", TZ_PARIS, 1);  // heure de Paris, été comme hiver, même sans réseau
  tzset();
  wifiInit();
  uiInit();
}

void loop() {
  wifiLoop();
  static uint32_t last = millis();
  uint32_t now = millis();
  lv_tick_inc(now - last);
  last = now;
  lv_timer_handler();
  delay(5);
}
