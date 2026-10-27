#include "wifi_mgr.h"
#include <WiFi.h>
#include <Preferences.h>
#include <time.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

constexpr const char* TZ_PARIS = "CET-1CEST,M3.5.0,M10.5.0/3";
constexpr int MAX_SAVED = 3, MAX_NETS = 5, MAX_SCAN = 12;

struct Net { String ssid, pass; };
static Net nets[MAX_NETS];
static int netCount = 0;
static int idx = 0;
static uint32_t lastTry = 0;
static bool first = true, wasUp = false, paused = false;

struct Found { String ssid; int rssi; bool open; };
static Found found[MAX_SCAN];
static int foundCount = -1;
static bool scanning = false;

static void loadNets() {
  netCount = 0;
  Preferences p;
  p.begin("wifi", true);
  int n = p.getInt("n", 0);
  for (int i = 0; i < n && netCount < MAX_SAVED; i++) {
    char ks[4], kp[4];
    snprintf(ks, sizeof ks, "s%d", i);
    snprintf(kp, sizeof kp, "p%d", i);
    nets[netCount++] = {p.getString(ks, ""), p.getString(kp, "")};
  }
  p.end();
#ifdef WIFI_SSID_1
  if (netCount < MAX_NETS) nets[netCount++] = {WIFI_SSID_1, WIFI_PASS_1};
#endif
#ifdef WIFI_SSID_2
  if (netCount < MAX_NETS) nets[netCount++] = {WIFI_SSID_2, WIFI_PASS_2};
#endif
}

void wifiInit() {
  WiFi.mode(WIFI_STA);
  loadNets();
}

bool wifiConnected() { return WiFi.status() == WL_CONNECTED; }
String wifiSsid() { return WiFi.SSID(); }

void wifiLoop() {
  bool up = wifiConnected();
  if (up && !wasUp) {
    Serial.printf("Wi-Fi connecte : %s\n", WiFi.SSID().c_str());
    configTzTime(TZ_PARIS, "pool.ntp.org", "time.google.com");
  }
  wasUp = up;
  if (scanning) {
    int n = WiFi.scanComplete();
    if (n >= 0) {
      foundCount = 0;
      for (int i = 0; i < n && foundCount < MAX_SCAN; i++) {
        String s = WiFi.SSID(i);
        if (s.length() == 0) continue;
        bool dup = false;
        for (int k = 0; k < foundCount; k++) if (found[k].ssid == s) dup = true;
        if (!dup) found[foundCount++] = {s, WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN};
      }
      for (int a = 1; a < foundCount; a++)  // du plus fort au plus faible
        for (int b = a; b > 0 && found[b].rssi > found[b - 1].rssi; b--) { Found t = found[b]; found[b] = found[b - 1]; found[b - 1] = t; }
      WiFi.scanDelete();
      scanning = false;
    } else if (n == WIFI_SCAN_FAILED) {
      foundCount = 0;
      scanning = false;
    }
    return;
  }
  if (paused || netCount == 0 || up) return;
  if (first || millis() - lastTry > 15000) {
    first = false;
    lastTry = millis();
    WiFi.disconnect();
    WiFi.begin(nets[idx].ssid.c_str(), nets[idx].pass.c_str());
    idx = (idx + 1) % netCount;
  }
}

void wifiScanStart() {
  paused = true;
  if (!wifiConnected()) WiFi.disconnect();
  foundCount = -1;
  scanning = true;
  WiFi.scanNetworks(true);
}

int wifiScanCount() { return scanning ? -1 : foundCount; }
const char* wifiScanSsid(int i) { return found[i].ssid.c_str(); }
int wifiScanRssi(int i) { return found[i].rssi; }
bool wifiScanOpen(int i) { return found[i].open; }

void wifiResume() {
  paused = false;
  first = true;
}

void wifiConnect(const char* ssid, const char* pass) {
  // Enregistre en tête de liste (le plus récent d'abord), 3 réseaux au plus
  Preferences p;
  p.begin("wifi", false);
  int n = p.getInt("n", 0);
  String ss[MAX_SAVED + 1], pp[MAX_SAVED + 1];
  int m = 0;
  ss[m] = ssid; pp[m] = pass; m++;
  for (int i = 0; i < n && m < MAX_SAVED; i++) {
    char ks[4], kp[4];
    snprintf(ks, sizeof ks, "s%d", i);
    snprintf(kp, sizeof kp, "p%d", i);
    String s = p.getString(ks, "");
    if (s.length() && s != ssid) { ss[m] = s; pp[m] = p.getString(kp, ""); m++; }
  }
  for (int i = 0; i < m; i++) {
    char ks[4], kp[4];
    snprintf(ks, sizeof ks, "s%d", i);
    snprintf(kp, sizeof kp, "p%d", i);
    p.putString(ks, ss[i]);
    p.putString(kp, pp[i]);
  }
  p.putInt("n", m);
  p.end();
  loadNets();
  paused = false;
  first = false;
  idx = netCount > 1 ? 1 : 0;  // le réseau choisi est en tête ; le suivant prendra le relais
  lastTry = millis();
  WiFi.disconnect();
  WiFi.begin(ssid, pass);
}

void wifiForgetAll() {
  Preferences p;
  p.begin("wifi", false);
  p.clear();
  p.end();
  WiFi.disconnect();
  loadNets();
  first = true;
}
