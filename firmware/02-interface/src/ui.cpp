// Interface de l'assistant : accueil, pointage, focus, vision, trajets.
// Toutes les valeurs venant de Home Assistant (alarme, pas, trajets) sont fictives à cette étape.
#include <Arduino.h>
#include <lvgl.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <esp_sntp.h>
#include "ui.h"
#include "wifi_mgr.h"

LV_FONT_DECLARE(font_ui_12);
LV_FONT_DECLARE(font_ui_14);
LV_FONT_DECLARE(font_ui_18);
LV_FONT_DECLARE(font_ui_26);
LV_FONT_DECLARE(font_num_20);
LV_FONT_DECLARE(font_num_30);
LV_FONT_DECLARE(font_num_52);

#define C_BG lv_color_hex(0x0A1019)
#define C_BAR lv_color_hex(0x0D1522)
#define C_CARD lv_color_hex(0x111B2B)
#define C_CARD2 lv_color_hex(0x16233A)
#define C_LINE lv_color_hex(0x1D2B40)
#define C_INK lv_color_hex(0xE8F1FA)
#define C_MUTED lv_color_hex(0x8196AD)
#define C_CYAN lv_color_hex(0x3DDCFF)
#define C_BLUE lv_color_hex(0x4C8DFF)
#define C_ALERT lv_color_hex(0xFF4D5E)
#define C_AMBER lv_color_hex(0xFFB020)
#define C_SUN lv_color_hex(0xFFD53D)
#define C_DARKTXT lv_color_hex(0x04111F)

enum Page { P_HOME, P_PUNCH, P_FOCUS, P_VISION, P_TRIPS, P_SETTIME, P_WIFI, P_SETTINGS, P_COUNT };
enum Expr { X_OPEN, X_HAPPY, X_FOCUS, X_SLEEPY, X_ALERT };

static lv_obj_t* pages[P_COUNT];
static lv_obj_t* tabBtn[4];
static lv_obj_t* tabLbl[4];
static lv_obj_t *buddyFace, *eyeL, *eyeR;
static Expr expr = X_OPEN;
static bool blinking = false;

// ---------- aides ----------

static void noScroll(lv_obj_t* o) { lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE); }

static lv_obj_t* lbl(lv_obj_t* parent, const char* text, const lv_font_t* font, lv_color_t col, int x, int y) {
  lv_obj_t* l = lv_label_create(parent);
  lv_label_set_text(l, text);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, col, 0);
  lv_obj_set_pos(l, x, y);
  return l;
}

static lv_obj_t* panel(lv_obj_t* parent, int x, int y, int w, int h, lv_color_t bg, lv_color_t border, int radius) {
  lv_obj_t* o = lv_obj_create(parent);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_color(o, bg, 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(o, border, 0);
  lv_obj_set_style_border_width(o, 1, 0);
  lv_obj_set_style_radius(o, radius, 0);
  lv_obj_set_style_pad_all(o, 0, 0);
  lv_obj_set_style_shadow_width(o, 0, 0);
  noScroll(o);
  return o;
}

static lv_obj_t* button(lv_obj_t* parent, int x, int y, int w, int h, const char* text, const lv_font_t* font,
                        lv_color_t bg, lv_color_t fg, lv_color_t border, lv_event_cb_t cb, void* user) {
  lv_obj_t* b = lv_btn_create(parent);
  lv_obj_set_pos(b, x, y);
  lv_obj_set_size(b, w, h);
  lv_obj_set_style_bg_color(b, bg, 0);
  lv_obj_set_style_bg_color(b, C_CARD2, LV_STATE_PRESSED);
  lv_obj_set_style_border_color(b, border, 0);
  lv_obj_set_style_border_width(b, 2, 0);
  lv_obj_set_style_radius(b, 12, 0);
  lv_obj_set_style_shadow_width(b, 0, 0);
  lv_obj_t* l = lv_label_create(b);
  lv_label_set_text(l, text);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, fg, 0);
  lv_obj_center(l);
  if (cb) lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, user);
  return b;
}

static lv_obj_t* newPage() {
  lv_obj_t* p = lv_obj_create(lv_scr_act());
  lv_obj_set_pos(p, 0, 0);
  lv_obj_set_size(p, 320, 196);
  lv_obj_set_style_bg_color(p, C_BG, 0);
  lv_obj_set_style_border_width(p, 0, 0);
  lv_obj_set_style_radius(p, 0, 0);
  lv_obj_set_style_pad_all(p, 0, 0);
  noScroll(p);
  lv_obj_add_flag(p, LV_OBJ_FLAG_HIDDEN);
  return p;
}

// ---------- message bref ----------

static lv_obj_t* toastObj;
static lv_obj_t* toastLbl;
static lv_timer_t* toastTimer = nullptr;

static void toastHide(lv_timer_t*) {
  lv_obj_add_flag(toastObj, LV_OBJ_FLAG_HIDDEN);
  toastTimer = nullptr;
}

static void toast(const char* text) {
  lv_label_set_text(toastLbl, text);
  lv_obj_clear_flag(toastObj, LV_OBJ_FLAG_HIDDEN);
  if (toastTimer) lv_timer_del(toastTimer);
  toastTimer = lv_timer_create(toastHide, 2200, nullptr);
  lv_timer_set_repeat_count(toastTimer, 1);
}

// ---------- compagnon ----------

static void buddyApply() {
  int h = 14, w = 9;
  lv_color_t col = C_CYAN;
  switch (expr) {
    case X_OPEN: h = 14; break;
    case X_HAPPY: h = 6; w = 11; break;
    case X_FOCUS: h = 9; break;
    case X_SLEEPY: h = 3; col = lv_color_hex(0x9A8A63); break;
    case X_ALERT: h = 17; col = C_ALERT; break;
  }
  if (blinking) h = 2;
  lv_obj_t* eyes[2] = {eyeL, eyeR};
  for (lv_obj_t* e : eyes) {
    lv_obj_set_size(e, w, h);
    lv_obj_set_style_bg_color(e, col, 0);
  }
}

static void buddySet(Expr e) {
  expr = e;
  buddyApply();
}

static void blinkEnd(lv_timer_t*) {
  blinking = false;
  buddyApply();
}

static void blinkStart(lv_timer_t*) {
  if (expr == X_SLEEPY || expr == X_HAPPY) return;
  blinking = true;
  buddyApply();
  lv_timer_t* t = lv_timer_create(blinkEnd, 130, nullptr);
  lv_timer_set_repeat_count(t, 1);
}

static void openSettings();
// Appui long mesuré ici plutôt que par LVGL : un léger tremblement du tactile ne l'annule pas.
static void onBuddyPress(lv_event_t* e) {
  static uint32_t start = 0;
  static bool fired = false;
  lv_event_code_t c = lv_event_get_code(e);
  if (c == LV_EVENT_PRESSED) {
    start = millis();
    fired = false;
    Serial.println("compagnon : appui");
  } else if (c == LV_EVENT_PRESSING && !fired && millis() - start > 600) {
    fired = true;
    Serial.println("compagnon : appui long -> reglages");
    openSettings();
  }
}

static void buddyCreate(lv_obj_t* parent, int x, int y) {
  buddyFace = panel(parent, x, y, 48, 34, lv_color_hex(0x0E1A2B), lv_color_hex(0x1E3550), 14);
  lv_obj_set_style_border_width(buddyFace, 2, 0);
  lv_obj_set_flex_flow(buddyFace, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(buddyFace, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(buddyFace, 10, 0);
  for (int i = 0; i < 2; i++) {
    lv_obj_t* e = lv_obj_create(buddyFace);
    lv_obj_set_style_bg_opa(e, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(e, 0, 0);
    lv_obj_set_style_radius(e, 5, 0);
    lv_obj_set_style_pad_all(e, 0, 0);
    noScroll(e);
    lv_obj_clear_flag(e, LV_OBJ_FLAG_CLICKABLE);
    (i == 0 ? eyeL : eyeR) = e;
  }
  lv_obj_set_style_border_color(buddyFace, C_CYAN, LV_STATE_PRESSED);
  lv_obj_set_ext_click_area(buddyFace, 12);  // zone tactile plus large que le dessin
  lv_obj_add_event_cb(buddyFace, onBuddyPress, LV_EVENT_PRESSED, nullptr);
  lv_obj_add_event_cb(buddyFace, onBuddyPress, LV_EVENT_PRESSING, nullptr);
  buddyApply();
  lv_timer_create(blinkStart, 4200, nullptr);
}

// ---------- navigation ----------

static bool wifiPausedByUi = false;

static void showPage(Page p) {
  if (p != P_WIFI && wifiPausedByUi) {
    wifiResume();
    wifiPausedByUi = false;
  }
  for (int i = 0; i < P_COUNT; i++) {
    if (i == p) lv_obj_clear_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
  }
  int active = p == P_SETTINGS || p == P_WIFI || p == P_SETTIME ? -1 : p == P_HOME || p == P_PUNCH ? 0 : p == P_FOCUS ? 1 : p == P_VISION ? 2 : 3;
  for (int i = 0; i < 4; i++) {
    lv_obj_set_style_text_color(tabLbl[i], i == active ? C_CYAN : C_MUTED, 0);
    lv_obj_set_style_border_width(tabBtn[i], i == active ? 2 : 0, 0);
  }
  if (p == P_HOME || p == P_PUNCH || p == P_SETTIME || p == P_WIFI || p == P_SETTINGS) buddySet(X_HAPPY);
  else if (p == P_FOCUS) buddySet(X_FOCUS);
  else buddySet(X_OPEN);
}

static void onTab(lv_event_t* e) {
  static const Page map[4] = {P_HOME, P_FOCUS, P_VISION, P_TRIPS};
  showPage(map[(intptr_t)lv_event_get_user_data(e)]);
}

// ---------- accueil ----------

static lv_obj_t *homeTime, *homeDate, *homeSrc, *homeWifi, *homePunch, *punchLast;

// ---------- heure : NTP par Wi-Fi, sinon réglage manuel ----------

static const char* DAYS[7] = {"dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"};
static const char* MONTHS[12] = {"janvier", "février", "mars", "avril", "mai", "juin",
                                 "juillet", "août", "septembre", "octobre", "novembre", "décembre"};
static volatile bool timeSynced = false;
static bool timeManual = false;

static void onSntpSync(struct timeval*) {
  timeSynced = true;
  timeManual = false;
}

static bool nowTm(struct tm& t) {
  time_t n = time(nullptr);
  localtime_r(&n, &t);
  return n > 1704067200;  // après le 1er janvier 2024 : l'heure a été réglée
}

static void fmtHM(char* b, size_t n) {
  struct tm t;
  if (nowTm(t)) snprintf(b, n, "%02d:%02d", t.tm_hour, t.tm_min);
  else snprintf(b, n, "--:--");
}

static void clockTick(lv_timer_t*) {
  char b[48];
  fmtHM(b, sizeof b);
  lv_label_set_text(homeTime, b);
  struct tm t;
  if (nowTm(t)) {
    snprintf(b, sizeof b, "%s %d %s", DAYS[t.tm_wday], t.tm_mday, MONTHS[t.tm_mon]);
    lv_label_set_text(homeDate, b);
    lv_label_set_text(homeSrc, timeSynced ? "" : "heure manuelle");
  } else {
    lv_label_set_text(homeDate, "Touche l'heure pour la régler");
    lv_label_set_text(homeSrc, "");
  }
  if (wifiConnected()) {
    String n = wifiSsid();
    if (n.length() > 16) n = n.substring(0, 15) + ".";
    lv_label_set_text(homeWifi, n.c_str());
    lv_obj_set_style_text_color(homeWifi, C_CYAN, 0);
  } else {
    lv_label_set_text(homeWifi, "Choisir le Wi-Fi");
    lv_obj_set_style_text_color(homeWifi, C_AMBER, 0);
  }
}

static struct tm editTm;
static lv_obj_t *stHour, *stMin, *stDate;

static void setTimeRefresh() {
  char b[48];
  snprintf(b, sizeof b, "%02d", editTm.tm_hour);
  lv_label_set_text(stHour, b);
  snprintf(b, sizeof b, "%02d", editTm.tm_min);
  lv_label_set_text(stMin, b);
  snprintf(b, sizeof b, "%s %d %s %d", DAYS[editTm.tm_wday], editTm.tm_mday, MONTHS[editTm.tm_mon], editTm.tm_year + 1900);
  lv_label_set_text(stDate, b);
}

static void openSetTime(lv_event_t*) {
  if (!nowTm(editTm)) {
    // Point de départ : la date de compilation du programme, à midi
    static const char* mon = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char m[4] = {0};
    int d = 1, y = 2026;
    sscanf(__DATE__, "%3s %d %d", m, &d, &y);
    memset(&editTm, 0, sizeof editTm);
    const char* f = strstr(mon, m);
    editTm.tm_mon = f ? (f - mon) / 3 : 0;
    editTm.tm_mday = d;
    editTm.tm_year = y - 1900;
    editTm.tm_hour = 12;
  }
  editTm.tm_sec = 0;
  editTm.tm_isdst = -1;
  mktime(&editTm);
  setTimeRefresh();
  showPage(P_SETTIME);
}

static void onAdjust(lv_event_t* e) {
  switch ((int)(intptr_t)lv_event_get_user_data(e)) {
    case 1: editTm.tm_hour = (editTm.tm_hour + 23) % 24; break;
    case 2: editTm.tm_hour = (editTm.tm_hour + 1) % 24; break;
    case 3: editTm.tm_min = (editTm.tm_min + 59) % 60; break;
    case 4: editTm.tm_min = (editTm.tm_min + 1) % 60; break;
    case 5: editTm.tm_mday -= 1; break;
    case 6: editTm.tm_mday += 1; break;
  }
  editTm.tm_isdst = -1;
  mktime(&editTm);  // recalcule le jour de la semaine et passe d'un mois à l'autre
  setTimeRefresh();
}

static void onSetTimeOk(lv_event_t*) {
  editTm.tm_sec = 0;
  editTm.tm_isdst = -1;
  struct timeval tv = {mktime(&editTm), 0};
  settimeofday(&tv, nullptr);
  timeSynced = false;
  timeManual = true;
  clockTick(nullptr);
  toast("Heure réglée");
  showPage(P_HOME);
}

static void onSetTimeCancel(lv_event_t*) { showPage(P_HOME); }

static lv_obj_t* adjButton(lv_obj_t* p, int x, int y, int w, int h, const char* t, int code) {
  lv_obj_t* b = button(p, x, y, w, h, t, &font_ui_26, C_CARD, C_INK, C_LINE, onAdjust, (void*)(intptr_t)code);
  lv_obj_add_event_cb(b, onAdjust, LV_EVENT_LONG_PRESSED_REPEAT, (void*)(intptr_t)code);
  return b;
}

static void buildSetTime() {
  lv_obj_t* p = pages[P_SETTIME];
  lbl(p, "Régler l'heure", &font_ui_14, C_MUTED, 14, 2);
  adjButton(p, 14, 24, 48, 56, "-", 1);
  stHour = lbl(p, "12", &font_num_30, C_INK, 62, 34);
  lv_obj_set_width(stHour, 48);
  lv_obj_set_style_text_align(stHour, LV_TEXT_ALIGN_CENTER, 0);
  adjButton(p, 110, 24, 48, 56, "+", 2);
  adjButton(p, 162, 24, 48, 56, "-", 3);
  stMin = lbl(p, "00", &font_num_30, C_INK, 210, 34);
  lv_obj_set_width(stMin, 48);
  lv_obj_set_style_text_align(stMin, LV_TEXT_ALIGN_CENTER, 0);
  adjButton(p, 258, 24, 48, 56, "+", 4);
  adjButton(p, 14, 90, 48, 48, "-", 5);
  stDate = lbl(p, "", &font_ui_14, C_CYAN, 62, 105);
  lv_obj_set_width(stDate, 196);
  lv_obj_set_style_text_align(stDate, LV_TEXT_ALIGN_CENTER, 0);
  adjButton(p, 258, 90, 48, 48, "+", 6);
  button(p, 14, 148, 140, 42, "Annuler", &font_ui_14, C_CARD, C_INK, C_LINE, onSetTimeCancel, nullptr);
  button(p, 166, 148, 140, 42, "Valider", &font_ui_18, C_CYAN, C_DARKTXT, C_CYAN, onSetTimeOk, nullptr);
}

// ---------- Wi-Fi : choix du réseau sur l'écran ----------

static lv_obj_t *wList, *wStatus, *pwOv, *pwTitle, *pwText;
static lv_timer_t* wTimer = nullptr;
static char wChosen[33];

static void wifiStatusRefresh() {
  if (wifiConnected()) {
    String t = String("Connecté à ") + wifiSsid();
    lv_label_set_text(wStatus, t.c_str());
  } else {
    lv_label_set_text(wStatus, "Non connecté");
  }
}

static void connectTo(const char* ssid, const char* pass) {
  wifiConnect(ssid, pass);
  wifiPausedByUi = false;
  char b[64];
  snprintf(b, sizeof b, "Connexion à %s...", ssid);
  toast(b);
  showPage(P_HOME);
}

static void onNetPick(lv_event_t* e) {
  int i = (int)(intptr_t)lv_event_get_user_data(e);
  strncpy(wChosen, wifiScanSsid(i), sizeof wChosen - 1);
  wChosen[sizeof wChosen - 1] = 0;
  if (wifiScanOpen(i)) { connectTo(wChosen, ""); return; }
  char b[64];
  snprintf(b, sizeof b, "Mot de passe : %s", wChosen);
  lv_label_set_text(pwTitle, b);
  lv_textarea_set_text(pwText, "");
  lv_obj_clear_flag(pwOv, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(pwOv);
}

static void wifiPoll(lv_timer_t*) {
  int n = wifiScanCount();
  if (n < 0) return;
  lv_timer_del(wTimer);
  wTimer = nullptr;
  lv_obj_clean(wList);
  if (n == 0) {
    lbl(wList, "Aucun réseau trouvé.\nSeul le Wi-Fi 2,4 GHz est visible.", &font_ui_14, C_MUTED, 0, 0);
    return;
  }
  for (int i = 0; i < n; i++) {
    int r = wifiScanRssi(i);
    char b[64];
    snprintf(b, sizeof b, "%s   %s%s", wifiScanSsid(i), r > -60 ? "fort" : r > -75 ? "moyen" : "faible", wifiScanOpen(i) ? " · ouvert" : "");
    lv_obj_t* bt = button(wList, 0, 0, 286, 38, b, &font_ui_14, C_CARD, C_INK, C_LINE, onNetPick, (void*)(intptr_t)i);
    lv_label_set_long_mode(lv_obj_get_child(bt, 0), LV_LABEL_LONG_DOT);
    lv_obj_set_width(lv_obj_get_child(bt, 0), 260);
  }
}

static void startScan() {
  lv_obj_clean(wList);
  lbl(wList, "Recherche des réseaux...", &font_ui_14, C_MUTED, 0, 0);
  wifiPausedByUi = true;
  wifiScanStart();
  if (!wTimer) wTimer = lv_timer_create(wifiPoll, 300, nullptr);
}

static void openWifi(lv_event_t*) {
  showPage(P_WIFI);
  wifiStatusRefresh();
  startScan();
}

static void onRescan(lv_event_t*) { startScan(); }
static void onWifiBack(lv_event_t*) { showPage(P_SETTINGS); }
static void onForget(lv_event_t*) {
  wifiForgetAll();
  wifiStatusRefresh();
  toast("Réseaux enregistrés oubliés");
}

static void onKeyboard(lv_event_t* e) {
  lv_event_code_t c = lv_event_get_code(e);
  if (c == LV_EVENT_READY) {
    lv_obj_add_flag(pwOv, LV_OBJ_FLAG_HIDDEN);
    connectTo(wChosen, lv_textarea_get_text(pwText));
    lv_textarea_set_text(pwText, "");
  } else if (c == LV_EVENT_CANCEL) {
    lv_obj_add_flag(pwOv, LV_OBJ_FLAG_HIDDEN);
    lv_textarea_set_text(pwText, "");
  }
}

static void buildWifi() {
  lv_obj_t* p = pages[P_WIFI];
  lbl(p, "Wi-Fi", &font_ui_14, C_INK, 12, 4);
  wStatus = lbl(p, "", &font_ui_12, C_MUTED, 64, 6);
  wList = lv_obj_create(p);
  lv_obj_set_pos(wList, 8, 28);
  lv_obj_set_size(wList, 304, 120);
  lv_obj_set_style_bg_color(wList, C_BG, 0);
  lv_obj_set_style_border_width(wList, 0, 0);
  lv_obj_set_style_pad_all(wList, 2, 0);
  lv_obj_set_style_pad_row(wList, 6, 0);
  lv_obj_set_flex_flow(wList, LV_FLEX_FLOW_COLUMN);
  button(p, 8, 154, 96, 38, "Rechercher", &font_ui_14, C_CARD, C_INK, C_LINE, onRescan, nullptr);
  button(p, 112, 154, 96, 38, "Oublier tout", &font_ui_14, C_CARD, C_MUTED, C_LINE, onForget, nullptr);
  button(p, 216, 154, 96, 38, "Retour", &font_ui_14, C_CARD, C_INK, C_BLUE, onWifiBack, nullptr);

  // Saisie du mot de passe : plein écran avec clavier
  pwOv = lv_obj_create(lv_scr_act());
  lv_obj_set_pos(pwOv, 0, 0);
  lv_obj_set_size(pwOv, 320, 240);
  lv_obj_set_style_bg_color(pwOv, C_BG, 0);
  lv_obj_set_style_border_width(pwOv, 0, 0);
  lv_obj_set_style_radius(pwOv, 0, 0);
  lv_obj_set_style_pad_all(pwOv, 0, 0);
  noScroll(pwOv);
  pwTitle = lbl(pwOv, "", &font_ui_14, C_MUTED, 8, 3);
  pwText = lv_textarea_create(pwOv);
  lv_obj_set_pos(pwText, 8, 22);
  lv_obj_set_size(pwText, 304, 36);
  lv_textarea_set_one_line(pwText, true);
  lv_textarea_set_password_mode(pwText, true);
  lv_textarea_set_placeholder_text(pwText, "mot de passe");
  lv_obj_set_style_bg_color(pwText, C_CARD, 0);
  lv_obj_set_style_text_color(pwText, C_INK, 0);
  lv_obj_set_style_border_color(pwText, C_CYAN, 0);
  lv_obj_t* kb = lv_keyboard_create(pwOv);
  lv_obj_set_pos(kb, 0, 62);
  lv_obj_set_size(kb, 320, 178);
  lv_keyboard_set_textarea(kb, pwText);
  lv_obj_set_style_bg_color(kb, C_BAR, 0);
  lv_obj_set_style_bg_color(kb, C_CARD2, LV_PART_ITEMS);
  lv_obj_set_style_text_color(kb, C_INK, LV_PART_ITEMS);
  lv_obj_add_event_cb(kb, onKeyboard, LV_EVENT_ALL, nullptr);
  lv_obj_add_flag(pwOv, LV_OBJ_FLAG_HIDDEN);
}

// ---------- réglages : appui long sur le compagnon ----------

static lv_obj_t* setWifiLbl;

static void openSettings() {
  if (wifiConnected()) {
    String t = String("Wi-Fi\n") + wifiSsid();
    lv_label_set_text(setWifiLbl, t.c_str());
  } else {
    lv_label_set_text(setWifiLbl, "Wi-Fi\nnon connecté");
  }
  showPage(P_SETTINGS);
}

static void onSetWifi(lv_event_t*) { openWifi(nullptr); }
static void onSetClock(lv_event_t*) { openSetTime(nullptr); }
static void onNextProject(lv_event_t*) { toast("Ce bouton servira au prochain projet"); }

static void buildSettings() {
  lv_obj_t* p = pages[P_SETTINGS];
  lbl(p, "Réglages", &font_ui_18, C_INK, 14, 4);
  lv_obj_t* v = lbl(p, "clin · v0.5", &font_ui_12, C_MUTED, 0, 0);
  lv_obj_align(v, LV_ALIGN_TOP_RIGHT, -14, 9);
  lv_obj_t* w = button(p, 12, 36, 144, 74, "Wi-Fi", &font_ui_14, C_CARD, C_INK, C_CYAN, onSetWifi, nullptr);
  setWifiLbl = lv_obj_get_child(w, 0);
  lv_obj_set_style_text_align(setWifiLbl, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_t* h = button(p, 164, 36, 144, 74, "Heure\nrégler à la main", &font_ui_14, C_CARD, C_INK, C_LINE, onSetClock, nullptr);
  lv_obj_set_style_text_align(lv_obj_get_child(h, 0), LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_t* n = button(p, 12, 118, 296, 70, "Prochain projet (à venir)", &font_ui_14, C_BG, C_MUTED, C_LINE, onNextProject, nullptr);
  lv_obj_set_style_border_opa(n, LV_OPA_70, 0);
}

static void onPunchCard(lv_event_t*) { showPage(P_PUNCH); }

static lv_obj_t* card(lv_obj_t* parent, int x, int w, const char* title, const char* value, const lv_font_t* vfont,
                      const char* sub, bool fake, lv_obj_t** valueOut) {
  lv_obj_t* c = panel(parent, x, 112, w, 76, C_CARD, C_LINE, 12);
  lbl(c, title, &font_ui_12, C_MUTED, 8, 6);
  lv_obj_t* v = lbl(c, value, vfont, C_INK, 8, 24);
  lbl(c, sub, &font_ui_12, fake ? C_AMBER : C_MUTED, 8, 52);
  if (valueOut) *valueOut = v;
  return c;
}

static void buildHome() {
  lv_obj_t* p = pages[P_HOME];
  lbl(p, "Bonjour, Dimitri", &font_ui_14, C_MUTED, 14, 2);
  homeTime = lbl(p, "--:--", &font_num_52, C_INK, 10, 28);
  homeDate = lbl(p, "", &font_ui_14, C_CYAN, 14, 84);
  homeSrc = lbl(p, "", &font_ui_12, C_AMBER, 0, 0);
  lv_obj_align(homeSrc, LV_ALIGN_TOP_RIGHT, -12, 87);
  lv_obj_t* wb = lv_btn_create(p);  // zone tactile en haut à droite : état du Wi-Fi, ouvre le choix du réseau
  lv_obj_set_pos(wb, 168, 0);
  lv_obj_set_size(wb, 152, 30);
  lv_obj_set_style_bg_opa(wb, LV_OPA_TRANSP, 0);
  lv_obj_set_style_shadow_width(wb, 0, 0);
  lv_obj_set_style_border_width(wb, 0, 0);
  lv_obj_add_event_cb(wb, openWifi, LV_EVENT_CLICKED, nullptr);
  homeWifi = lbl(wb, "", &font_ui_12, C_AMBER, 0, 0);
  lv_obj_align(homeWifi, LV_ALIGN_RIGHT_MID, -2, 0);
  lv_obj_t* tapTargets[2] = {homeTime, homeDate};  // toucher l'heure ou la date ouvre le réglage
  for (lv_obj_t* o : tapTargets) {
    lv_obj_add_flag(o, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(o, openSetTime, LV_EVENT_CLICKED, nullptr);
  }
  lv_obj_t* c1 = card(p, 12, 146, "POINTAGE", "08:03", &font_num_20, "Arrivée", false, &homePunch);
  lv_obj_add_flag(c1, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(c1, onPunchCard, LV_EVENT_CLICKED, nullptr);
  card(p, 162, 146, "PAS", "4 210", &font_num_20, "fictif", true, nullptr);
  lv_timer_create(clockTick, 1000, nullptr);
  clockTick(nullptr);
}

// ---------- pointage ----------

static void onPunch(lv_event_t* e) {
  bool arrival = (intptr_t)lv_event_get_user_data(e) == 1;
  struct tm t;
  if (!nowTm(t)) {
    toast("Règle d'abord l'heure");
    openSetTime(nullptr);
    return;
  }
  char b[48];
  snprintf(b, sizeof b, "%s · %02d:%02d", arrival ? "Arrivée enregistrée" : "Départ enregistré", t.tm_hour, t.tm_min);
  toast(b);
  snprintf(b, sizeof b, "Dernier : %s %02d:%02d", arrival ? "arrivée" : "départ", t.tm_hour, t.tm_min);
  lv_label_set_text(punchLast, b);
  snprintf(b, sizeof b, "%02d:%02d", t.tm_hour, t.tm_min);
  lv_label_set_text(homePunch, b);
}

static void onSoon(lv_event_t*) { toast("Disponible à l'étape suivante"); }

static void buildPunch() {
  lv_obj_t* p = pages[P_PUNCH];
  punchLast = lbl(p, "Dernier : arrivée 08:03", &font_ui_14, C_MUTED, 14, 4);
  button(p, 12, 28, 144, 86, "ARRIVÉE", &font_ui_18, C_CYAN, C_DARKTXT, C_CYAN, onPunch, (void*)1);
  button(p, 164, 28, 144, 86, "DÉPART", &font_ui_18, C_CARD, C_INK, C_BLUE, onPunch, (void*)2);
  button(p, 12, 126, 144, 44, "Historique", &font_ui_14, C_CARD, C_INK, C_LINE, onSoon, nullptr);
  button(p, 164, 126, 144, 44, "Corriger", &font_ui_14, C_CARD, C_INK, C_LINE, onSoon, nullptr);
}

// ---------- focus ----------

enum FState { F_IDLE, F_RUN, F_PAUSED };
static FState fState = F_IDLE;
static bool fFocusPhase = true;
static int fPreset = 0;  // 0 = 25/5, 1 = 50/10
static uint32_t fRemainMs = 25UL * 60000UL, fTotalMs = 25UL * 60000UL, fLast = 0;
static lv_obj_t *fArc, *fTime, *fPhase, *fMain, *fMainLbl, *fChip[2];

static uint32_t focusMs() { return (fPreset ? 50UL : 25UL) * 60000UL; }
static uint32_t breakMs() { return (fPreset ? 10UL : 5UL) * 60000UL; }

static void focusRefresh() {
  uint32_t s = (fRemainMs + 999) / 1000;
  char b[8];
  snprintf(b, sizeof b, "%02u:%02u", (unsigned)(s / 60), (unsigned)(s % 60));
  lv_label_set_text(fTime, b);
  lv_arc_set_value(fArc, fTotalMs ? (int)((uint64_t)fRemainMs * 1000 / fTotalMs) : 0);
  lv_label_set_text(fPhase, fFocusPhase ? "CONCENTRATION" : "PAUSE");
  lv_label_set_text(fMainLbl, fState == F_RUN ? "Pause" : fState == F_PAUSED ? "Reprendre" : "Démarrer");
}

static void focusReset() {
  fState = F_IDLE;
  fFocusPhase = true;
  fTotalMs = fRemainMs = focusMs();
  focusRefresh();
}

static void focusTick(lv_timer_t*) {
  uint32_t now = millis();
  if (fState == F_RUN) {
    uint32_t dt = now - fLast;
    if (dt >= fRemainMs) {
      fFocusPhase = !fFocusPhase;
      fTotalMs = fRemainMs = fFocusPhase ? focusMs() : breakMs();
      toast(fFocusPhase ? "Retour à la concentration" : "Pause : lève les yeux");
    } else {
      fRemainMs -= dt;
    }
    focusRefresh();
  }
  fLast = now;
}

static void onFocusMain(lv_event_t*) {
  fLast = millis();
  fState = fState == F_RUN ? F_PAUSED : F_RUN;
  focusRefresh();
}

static void onFocusStop(lv_event_t*) { focusReset(); }

static void onPreset(lv_event_t* e) {
  fPreset = (intptr_t)lv_event_get_user_data(e);
  for (int i = 0; i < 2; i++) {
    lv_obj_set_style_border_color(fChip[i], i == fPreset ? C_CYAN : C_LINE, 0);
    lv_obj_set_style_text_color(lv_obj_get_child(fChip[i], 0), i == fPreset ? C_CYAN : C_MUTED, 0);
  }
  focusReset();
}

static void buildFocus() {
  lv_obj_t* p = pages[P_FOCUS];
  fArc = lv_arc_create(p);
  lv_obj_set_size(fArc, 132, 132);
  lv_obj_set_pos(fArc, 12, 28);
  lv_arc_set_rotation(fArc, 270);
  lv_arc_set_bg_angles(fArc, 0, 360);
  lv_arc_set_range(fArc, 0, 1000);
  lv_obj_remove_style(fArc, nullptr, LV_PART_KNOB);
  lv_obj_clear_flag(fArc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(fArc, 9, LV_PART_MAIN);
  lv_obj_set_style_arc_width(fArc, 9, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(fArc, C_CARD2, LV_PART_MAIN);
  lv_obj_set_style_arc_color(fArc, C_CYAN, LV_PART_INDICATOR);
  fTime = lbl(p, "25:00", &font_num_30, C_INK, 0, 0);
  lv_obj_align_to(fTime, fArc, LV_ALIGN_CENTER, 0, -6);
  lv_obj_t* r = lbl(p, "RESTANT", &font_ui_12, C_MUTED, 0, 0);
  lv_obj_align_to(r, fArc, LV_ALIGN_CENTER, 0, 22);
  fPhase = lbl(p, "CONCENTRATION", &font_ui_12, C_MUTED, 162, 8);
  const char* names[2] = {"25/5", "50/10"};
  for (int i = 0; i < 2; i++) {
    fChip[i] = button(p, 162 + i * 78, 30, 70, 40, names[i], &font_ui_14, C_CARD, i == 0 ? C_CYAN : C_MUTED,
                      i == 0 ? C_CYAN : C_LINE, onPreset, (void*)(intptr_t)i);
  }
  fMain = button(p, 162, 80, 146, 54, "Démarrer", &font_ui_18, C_CARD2, C_INK, C_BLUE, onFocusMain, nullptr);
  fMainLbl = lv_obj_get_child(fMain, 0);
  button(p, 162, 142, 146, 40, "Arrêter", &font_ui_14, C_CARD, C_MUTED, C_LINE, onFocusStop, nullptr);
  lv_timer_create(focusTick, 250, nullptr);
  focusReset();
}

// ---------- vision ----------

struct Exercise { const char* name; const char* title; uint16_t seconds; };
static const Exercise EX[8] = {
  {"20-20-20", "Règle 20-20-20", 20},
  {"Clignements", "Clignements guidés", 30},
  {"Près / loin", "Près et loin", 40},
  {"Suivi lent", "Suivi lent", 45},
  {"Points alternés", "Points alternés", 40},
  {"Huit", "Mouvement en huit", 60},
  {"Respiration", "Pause active", 90},
  {"Au hasard", "Au hasard", 0},
};

static lv_obj_t *vOv, *vTitle, *vTime, *vText, *vDot, *vRing, *vHint;
static lv_timer_t* vTimer = nullptr;
static int vType = 0;
static uint32_t vStart = 0, vDur = 0;

static void dotAt(float x, float y, int size) {
  lv_obj_set_size(vDot, size, size);
  lv_obj_set_pos(vDot, (int)(160 + x - size / 2), (int)(96 + y - size / 2));
}

static void ringSize(int size) {
  lv_obj_set_size(vRing, size, size);
  lv_obj_set_pos(vRing, 160 - size / 2, 96 - size / 2);
}

static void visionEnd(bool done) {
  if (vTimer) { lv_timer_del(vTimer); vTimer = nullptr; }
  lv_obj_add_flag(vOv, LV_OBJ_FLAG_HIDDEN);
  buddySet(done ? X_HAPPY : X_OPEN);
  if (done) toast("Exercice terminé");
}

static void visionTick(lv_timer_t*) {
  float t = (millis() - vStart) / 1000.0f;
  if (t >= vDur) { visionEnd(true); return; }
  char b[8];
  uint32_t rem = vDur - (uint32_t)t;
  snprintf(b, sizeof b, "%u:%02u", (unsigned)(rem / 60), (unsigned)(rem % 60));
  lv_label_set_text(vTime, b);
  switch (vType) {
    case 0: break;
    case 1: {  // un cercle qui grossit toutes les 3 s : un clignement doux
      float p = fmodf(t, 3.0f);
      ringSize(p < 0.6f ? 50 + (int)(50 * p / 0.6f) : 100 - (int)(50 * (p - 0.6f) / 2.4f));
      lv_label_set_text(vText, p < 1.0f ? "Cligne doucement" : "Détends tes yeux");
      break;
    }
    case 2: {
      bool near = ((int)(t / 5.0f)) % 2 == 0;
      dotAt(0, 0, near ? 36 : 8);
      lv_label_set_text(vText, near ? "Regarde près" : "Regarde au loin");
      break;
    }
    case 3: dotAt(sinf(t * 1.1f) * 120, 0, 16); break;
    case 4: dotAt(((int)(t / 0.8f)) % 2 ? 115 : -115, 0, 16); break;
    case 5: {
      float u = t * 1.3f;
      dotAt(sinf(u) * 120, sinf(2 * u) * 38, 16);
      break;
    }
    case 6: {  // 4 s d'inspiration, 6 s d'expiration
      float p = fmodf(t, 10.0f);
      ringSize(p < 4.0f ? 40 + (int)(60 * p / 4.0f) : 100 - (int)(60 * (p - 4.0f) / 6.0f));
      lv_label_set_text(vText, p < 4.0f ? "Inspire" : "Expire");
      break;
    }
  }
}

static void visionStart(int idx) {
  if (idx == 7) idx = (int)random(1, 7);
  vType = idx;
  vDur = EX[idx].seconds;
  vStart = millis();
  lv_label_set_text(vTitle, EX[idx].title);
  lv_label_set_text(vText, "");
  lv_label_set_text(vHint, "");
  lv_obj_add_flag(vDot, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(vRing, LV_OBJ_FLAG_HIDDEN);
  switch (idx) {
    case 0:
      lv_label_set_text(vText, "Regarde un objet à 6 mètres\npendant 20 secondes");
      lv_obj_set_y(vText, 74);
      break;
    case 1: case 6:
      lv_obj_clear_flag(vRing, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_y(vText, 150);
      if (idx == 6) lv_label_set_text(vHint, "Relâche les épaules");
      break;
    case 2:
      lv_obj_clear_flag(vDot, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_y(vText, 150);
      break;
    default:
      lv_obj_clear_flag(vDot, LV_OBJ_FLAG_HIDDEN);
      break;
  }
  lv_obj_clear_flag(vOv, LV_OBJ_FLAG_HIDDEN);
  if (vTimer) lv_timer_del(vTimer);
  vTimer = lv_timer_create(visionTick, 30, nullptr);
}

static void onVisionPick(lv_event_t* e) { visionStart((int)(intptr_t)lv_event_get_user_data(e)); }
static void onVisionStop(lv_event_t*) { visionEnd(false); }

static void buildVision() {
  lv_obj_t* p = pages[P_VISION];
  for (int i = 0; i < 8; i++) {
    int x = 8 + (i % 2) * 156, y = 4 + (i / 2) * 48;
    button(p, x, y, 148, 42, EX[i].name, &font_ui_14, C_CARD, C_INK, C_LINE, onVisionPick, (void*)(intptr_t)i);
  }
  // Écran d'exercice : sombre, plein écran, arrêt immédiat
  vOv = lv_obj_create(lv_scr_act());
  lv_obj_set_pos(vOv, 0, 0);
  lv_obj_set_size(vOv, 320, 240);
  lv_obj_set_style_bg_color(vOv, lv_color_hex(0x04060A), 0);
  lv_obj_set_style_border_width(vOv, 0, 0);
  lv_obj_set_style_radius(vOv, 0, 0);
  lv_obj_set_style_pad_all(vOv, 0, 0);
  noScroll(vOv);
  lv_obj_add_flag(vOv, LV_OBJ_FLAG_HIDDEN);
  vTitle = lbl(vOv, "", &font_ui_18, lv_color_hex(0xF5E6A8), 14, 6);
  vTime = lbl(vOv, "", &font_num_20, C_MUTED, 262, 8);
  vRing = panel(vOv, 0, 0, 50, 50, lv_color_hex(0x04060A), C_SUN, LV_RADIUS_CIRCLE);
  lv_obj_set_style_border_width(vRing, 3, 0);
  vDot = panel(vOv, 0, 0, 16, 16, C_SUN, C_SUN, LV_RADIUS_CIRCLE);
  vText = lbl(vOv, "", &font_ui_18, C_INK, 0, 150);
  vHint = lbl(vOv, "", &font_ui_14, C_MUTED, 0, 174);
  lv_obj_t* centred[2] = {vText, vHint};
  for (lv_obj_t* t : centred) {
    lv_obj_set_width(t, 320);
    lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, 0);
  }
  button(vOv, 12, 194, 132, 40, "Arrêter", &font_ui_14, lv_color_hex(0x151A24), C_INK, lv_color_hex(0x3A4659), onVisionStop, nullptr);
  lv_obj_t* d = lbl(vOv, "Confort visuel,\npas un avis médical.", &font_ui_12, lv_color_hex(0x6C7C90), 156, 198);
  lv_obj_set_width(d, 152);
  lv_label_set_long_mode(d, LV_LABEL_LONG_WRAP);
}

// ---------- trajets (valeurs fictives) ----------

struct Trip { const char* name; const char* note; int minutes; bool slow; };
static const Trip TRIPS[3] = {
  {"Domicile", "Trafic fluide", 24, false},
  {"Chez Ornella", "Ralentissement +6 min", 31, true},
  {"Récupérer les enfants", "Trafic fluide", 18, false},
};

static void buildTrips() {
  lv_obj_t* p = pages[P_TRIPS];
  lbl(p, "Temps de trajet", &font_ui_14, C_INK, 14, 3);
  lv_obj_t* upd = lbl(p, "fictif · MàJ 08:41", &font_ui_12, C_AMBER, 0, 0);
  lv_obj_align(upd, LV_ALIGN_TOP_RIGHT, -12, 6);
  int best = 0;
  for (int i = 1; i < 3; i++) if (TRIPS[i].minutes < TRIPS[best].minutes) best = i;
  for (int i = 0; i < 3; i++) {
    int y = 26 + i * 56;
    bool isBest = i == best;
    lv_obj_t* r = panel(p, 12, y, 296, 52, isBest ? lv_color_hex(0x0F2030) : C_CARD, isBest ? C_CYAN : C_LINE, 12);
    lbl(r, TRIPS[i].name, &font_ui_14, C_INK, 10, 4);
    lbl(r, TRIPS[i].note, &font_ui_12, TRIPS[i].slow ? C_AMBER : C_MUTED, 10, 24);
    char b[8];
    snprintf(b, sizeof b, "%d", TRIPS[i].minutes);
    lv_obj_t* m = lbl(r, b, &font_num_30, isBest ? C_CYAN : C_INK, 0, 2);
    lv_obj_align(m, LV_ALIGN_TOP_RIGHT, -44, 10);
    lv_obj_t* u = lbl(r, "min", &font_ui_12, C_MUTED, 0, 0);
    lv_obj_align(u, LV_ALIGN_TOP_RIGHT, -12, 24);
    lv_obj_t* bar = lv_bar_create(r);
    lv_obj_set_size(bar, 276, 4);
    lv_obj_set_pos(bar, 10, 42);
    lv_bar_set_range(bar, 0, 40);
    lv_bar_set_value(bar, TRIPS[i].minutes, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, C_CARD2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, isBest ? C_CYAN : C_BLUE, LV_PART_INDICATOR);
  }
}

// ---------- veille : le compagnon seul, quand rien ne tourne ----------

constexpr uint32_t IDLE_MS = 60000;          // 1 min sans toucher l'écran
constexpr uint32_t DEEP_MS = 5UL * 60000;    // puis l'écran s'éteint complètement au bout de 5 min
enum SExpr { S_OPEN, S_LEFT, S_RIGHT, S_HAPPY, S_SURPRISED, S_SLEEP, S_CLOSED };

static lv_obj_t *sOv, *sEye[2], *sSmile, *sOh, *sZ, *sClock, *sName, *sTag, *sBar;
static bool booting = false;
static bool sleeping = false;
static SExpr sExpr = S_OPEN;
static uint32_t sSince = 0, sNextExpr = 0, sNextBlink = 0, sBlinkEnd = 0;
static int sLastKey = -1;

static void saverApply(bool blink) {
  int key = sExpr * 2 + (blink ? 1 : 0);
  if (key == sLastKey) return;
  sLastKey = key;
  int w = 26, h = 40, dx = 0;
  bool smile = false, oh = false;
  switch (sExpr) {
    case S_OPEN: break;
    case S_LEFT: dx = -14; break;
    case S_RIGHT: dx = 14; break;
    case S_HAPPY: h = 14; smile = true; break;
    case S_SURPRISED: w = 28; h = 50; oh = true; break;
    case S_SLEEP: h = 5; break;
    case S_CLOSED: h = 4; break;
  }
  if (blink && sExpr != S_SLEEP) h = 4;
  lv_color_t col = sExpr == S_SLEEP ? lv_color_hex(0x9A8A63) : C_CYAN;
  for (int i = 0; i < 2; i++) {
    int cx = (i == 0 ? 39 : 103) + dx;  // centres des yeux dans le visage (142 x 96 utiles)
    lv_obj_set_size(sEye[i], w, h);
    lv_obj_set_pos(sEye[i], cx - w / 2, 40 - h / 2);
    lv_obj_set_style_bg_color(sEye[i], col, 0);
  }
  if (smile) lv_obj_clear_flag(sSmile, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(sSmile, LV_OBJ_FLAG_HIDDEN);
  if (oh) lv_obj_clear_flag(sOh, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(sOh, LV_OBJ_FLAG_HIDDEN);
  if (sExpr == S_SLEEP) lv_obj_clear_flag(sZ, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(sZ, LV_OBJ_FLAG_HIDDEN);
}

static void saverClock() {
  char b[8];
  fmtHM(b, sizeof b);
  lv_label_set_text(sClock, b);
}

static void saverEnter() {
  sleeping = true;
  sSince = millis();
  sExpr = S_OPEN;
  sLastKey = -1;
  sNextExpr = sSince + 3000;
  sNextBlink = sSince + 2000;
  saverClock();
  saverApply(false);
  lv_obj_set_style_text_color(sName, C_MUTED, 0);
  lv_obj_clear_flag(sClock, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(sTag, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(sBar, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(sOv, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(sOv);
  backlight(90);
}

static void onSaverTouch(lv_event_t*) {
  if (booting) return;
  sleeping = false;
  lv_obj_add_flag(sOv, LV_OBJ_FLAG_HIDDEN);
  backlight(255);
  buddySet(X_HAPPY);
}

static void saverTick(lv_timer_t*) {
  uint32_t now = millis();
  if (booting) {  // démarrage : yeux fermés, ils s'ouvrent, un clignement, un sourire
    uint32_t t = now - sSince;
    sExpr = t < 900 ? S_CLOSED : t < 2000 ? S_OPEN : S_HAPPY;
    saverApply(t > 1400 && t < 1540);
    uint32_t pc = t * 100 / 2800;
    lv_bar_set_value(sBar, pc > 100 ? 100 : (int)pc, LV_ANIM_OFF);
    if (t > 2800) {
      booting = false;
      lv_obj_add_flag(sOv, LV_OBJ_FLAG_HIDDEN);
    }
    return;
  }
  if (!sleeping) {
    bool busy = fState == F_RUN || !lv_obj_has_flag(vOv, LV_OBJ_FLAG_HIDDEN) || !lv_obj_has_flag(pwOv, LV_OBJ_FLAG_HIDDEN);
    if (!busy && lv_disp_get_inactive_time(nullptr) > IDLE_MS) saverEnter();
    return;
  }
  if (sExpr != S_SLEEP && now - sSince > DEEP_MS) {
    sExpr = S_SLEEP;
    backlight(0);  // veille complète ; un appui rallume
  }
  if (sExpr != S_SLEEP && now >= sNextExpr) {
    int r = random(100);
    sExpr = r < 35 ? S_OPEN : r < 55 ? S_LEFT : r < 75 ? S_RIGHT : r < 95 ? S_HAPPY : S_SURPRISED;
    sNextExpr = now + random(3000, 7000);
  }
  if (now >= sNextBlink) {
    sBlinkEnd = now + 140;
    sNextBlink = now + random(2500, 6000);
  }
  saverApply(now < sBlinkEnd);
  if (sExpr == S_SLEEP) {
    static const char* zz[3] = {"z", "z  z", "z  z  z"};
    lv_label_set_text(sZ, zz[(now / 900) % 3]);
  }
  static uint32_t lastClock = 0;
  if (now - lastClock > 1000) { lastClock = now; saverClock(); }
}

static void buildSaver() {
  sOv = lv_obj_create(lv_scr_act());
  lv_obj_set_pos(sOv, 0, 0);
  lv_obj_set_size(sOv, 320, 240);
  lv_obj_set_style_bg_color(sOv, lv_color_hex(0x05080E), 0);
  lv_obj_set_style_border_width(sOv, 0, 0);
  lv_obj_set_style_radius(sOv, 0, 0);
  lv_obj_set_style_pad_all(sOv, 0, 0);
  noScroll(sOv);
  lv_obj_add_event_cb(sOv, onSaverTouch, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* face = panel(sOv, 85, 44, 150, 104, lv_color_hex(0x0E1A2B), lv_color_hex(0x1E3550), 40);
  lv_obj_set_style_border_width(face, 4, 0);
  lv_obj_clear_flag(face, LV_OBJ_FLAG_CLICKABLE);
  for (int i = 0; i < 2; i++) {
    sEye[i] = panel(face, 0, 0, 26, 40, C_CYAN, C_CYAN, 13);
    lv_obj_set_style_border_width(sEye[i], 0, 0);
    lv_obj_clear_flag(sEye[i], LV_OBJ_FLAG_CLICKABLE);
  }
  sSmile = lv_arc_create(face);
  lv_obj_set_size(sSmile, 44, 44);
  lv_obj_set_pos(sSmile, 49, 38);
  lv_arc_set_bg_angles(sSmile, 0, 0);
  lv_arc_set_angles(sSmile, 30, 150);
  lv_obj_remove_style(sSmile, nullptr, LV_PART_KNOB);
  lv_obj_clear_flag(sSmile, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_opa(sSmile, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_arc_width(sSmile, 5, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(sSmile, C_CYAN, LV_PART_INDICATOR);
  sOh = panel(face, 63, 70, 16, 16, lv_color_hex(0x0E1A2B), C_CYAN, LV_RADIUS_CIRCLE);
  lv_obj_set_style_border_width(sOh, 3, 0);
  lv_obj_clear_flag(sOh, LV_OBJ_FLAG_CLICKABLE);
  sZ = lbl(sOv, "z", &font_ui_18, lv_color_hex(0x9A8A63), 238, 22);
  sName = lbl(sOv, "clin", &font_ui_26, C_CYAN, 0, 154);
  sTag = lbl(sOv, "ton compagnon de bureau", &font_ui_12, C_MUTED, 0, 190);
  sClock = lbl(sOv, "--:--", &font_num_20, C_MUTED, 0, 194);
  lv_obj_t* centred[3] = {sName, sTag, sClock};
  for (lv_obj_t* o : centred) {
    lv_obj_set_width(o, 320);
    lv_obj_set_style_text_align(o, LV_TEXT_ALIGN_CENTER, 0);
  }
  lv_obj_set_style_text_letter_space(sName, 6, 0);
  sBar = lv_bar_create(sOv);
  lv_obj_set_pos(sBar, 100, 218);
  lv_obj_set_size(sBar, 120, 4);
  lv_bar_set_range(sBar, 0, 100);
  lv_obj_set_style_bg_color(sBar, C_CARD2, LV_PART_MAIN);
  lv_obj_set_style_bg_color(sBar, C_CYAN, LV_PART_INDICATOR);
  lv_obj_add_flag(sOv, LV_OBJ_FLAG_HIDDEN);
  lv_timer_create(saverTick, 60, nullptr);
}

static void bootStart() {
  booting = true;
  sSince = millis();
  sLastKey = -1;
  sExpr = S_CLOSED;
  saverApply(false);
  lv_obj_set_style_text_color(sName, C_CYAN, 0);
  lv_obj_add_flag(sClock, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(sTag, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(sBar, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(sZ, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(sOv, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(sOv);
}

// ---------- assemblage ----------

void uiInit() {
  lv_obj_set_style_bg_color(lv_scr_act(), C_BG, 0);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
  for (int i = 0; i < P_COUNT; i++) pages[i] = newPage();
  buildHome();
  buildPunch();
  buildFocus();
  buildVision();
  buildTrips();
  buildSetTime();
  buildWifi();
  buildSettings();
  sntp_set_time_sync_notification_cb(onSntpSync);

  lv_obj_t* bar = panel(lv_scr_act(), 0, 196, 320, 44, C_BAR, C_LINE, 0);
  lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_TOP, 0);
  const char* names[4] = {"Accueil", "Focus", "Vision", "Trajets"};
  const int xs[4] = {0, 66, 188, 254};
  for (int i = 0; i < 4; i++) {
    tabBtn[i] = lv_btn_create(bar);
    lv_obj_set_pos(tabBtn[i], xs[i], 0);
    lv_obj_set_size(tabBtn[i], 66, 43);
    lv_obj_set_style_bg_opa(tabBtn[i], LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(tabBtn[i], 0, 0);
    lv_obj_set_style_radius(tabBtn[i], 0, 0);
    lv_obj_set_style_border_color(tabBtn[i], C_CYAN, 0);
    lv_obj_set_style_border_side(tabBtn[i], LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_width(tabBtn[i], 0, 0);
    lv_obj_add_event_cb(tabBtn[i], onTab, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    tabLbl[i] = lv_label_create(tabBtn[i]);
    lv_label_set_text(tabLbl[i], names[i]);
    lv_obj_set_style_text_font(tabLbl[i], &font_ui_12, 0);
    lv_obj_center(tabLbl[i]);
  }
  buddyCreate(bar, 136, 4);
  lv_obj_move_foreground(vOv);  // l'écran d'exercice recouvre aussi la barre d'onglets
  lv_obj_move_foreground(pwOv);

  toastObj = panel(lv_scr_act(), 14, 150, 292, 34, C_INK, C_INK, 10);
  toastLbl = lbl(toastObj, "", &font_ui_14, C_DARKTXT, 0, 0);
  lv_obj_center(toastLbl);
  lv_obj_add_flag(toastObj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(toastObj);

  buildSaver();

  showPage(P_HOME);
  bootStart();
}
