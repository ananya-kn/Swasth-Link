// SwasthLink handheld: SpO2 + pulse (MAX30102), forehead temperature (MLX90614), OLED, BLE to the worker phone, SMS alert over SIM800L.
// Libraries: SparkFun MAX3010x, Adafruit MLX90614, Adafruit SSD1306 + GFX, TinyGPSPlus. (BLE + LittleFS ship with the ESP32 core.)
// STATUS: triage logic is test-verified (tests/run_all.sh). Sensor, BLE and GSM code is NOT yet run on hardware: bring up one block at a time.
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <TinyGPSPlus.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <LittleFS.h>
#include "config.h"
#include "sl_triage.h"

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
Adafruit_MLX90614 mlx;
MAX30105 ppg;
TinyGPSPlus gps;
BLECharacteristic *chr;
bool bleConnected = false;
uint32_t irBuf[100], redBuf[100];
int caseNo = 0;

class SrvCb : public BLEServerCallbacks {
  void onConnect(BLEServer *) { bleConnected = true; }
  void onDisconnect(BLEServer *s) { bleConnected = false; s->startAdvertising(); }
};

void show(const char *l1, const char *l2 = "", const char *l3 = "") {
  oled.clearDisplay(); oled.setTextColor(SSD1306_WHITE); oled.setTextSize(1);
  oled.setCursor(0, 0); oled.println(l1); oled.setCursor(0, 20); oled.println(l2); oled.setCursor(0, 40); oled.println(l3); oled.display();
}
void beep(int n, int ms = 120) { for (int i = 0; i < n; i++) { digitalWrite(PIN_BUZZER, HIGH); delay(ms); digitalWrite(PIN_BUZZER, LOW); delay(ms); } }

bool at(const char *cmd, const char *expect, uint32_t timeout = 2000) {
  Serial2.println(cmd); uint32_t t = millis(); String r;
  while (millis() - t < timeout) { while (Serial2.available()) r += (char)Serial2.read(); if (r.indexOf(expect) >= 0) return true; }
  return false;
}
bool sendSMS(const char *num, const String &text) {
  if (!at("AT", "OK") || !at("AT+CMGF=1", "OK")) return false;
  String c = String("AT+CMGS=\"") + num + "\""; if (!at(c.c_str(), ">", 5000)) return false;
  Serial2.print(text.substring(0, 158)); Serial2.write(26);
  return at("", "+CMGS", 15000);
}
String locText() { return gps.location.isValid() ? String(gps.location.lat(), 5) + "," + String(gps.location.lng(), 5) : String("noloc"); }
void alertPHC(const String &msg) { sendSMS(PHC_NUMBER_1, msg); sendSMS(PHC_NUMBER_2, msg); }

void logCase(const String &line) { File f = LittleFS.open("/cases.csv", FILE_APPEND); if (f) { f.println(line); f.close(); } }

void notifyBLE(float spo2, float hr, float temp, bool ok) {
  if (!bleConnected) return;
  char js[120]; snprintf(js, sizeof js, "{\"id\":\"%s-%d\",\"spo2\":%s,\"pulse\":%s,\"temp_c\":%s,\"sq\":%d}", DEVICE_ID, caseNo,
    isnan(spo2) ? "null" : String(spo2, 0).c_str(), isnan(hr) ? "null" : String(hr, 0).c_str(), isnan(temp) ? "null" : String(temp, 1).c_str(), ok ? 1 : 0);
  chr->setValue((uint8_t *)js, strlen(js)); chr->notify();
}

void measure() {
  show("Place finger", "on the sensor"); uint32_t t0 = millis();
  while (ppg.getIR() < FINGER_IR_MIN) { if (millis() - t0 > 15000) { show("No finger", "detected"); beep(2); return; } delay(50); }
  show("Hold still...", "reading SpO2");
  for (int i = 0; i < 100; i++) { while (!ppg.available()) ppg.check(); redBuf[i] = ppg.getRed(); irBuf[i] = ppg.getIR(); ppg.nextSample(); }
  int32_t spo2 = 0, hr = 0; int8_t vS = 0, vH = 0;
  for (int rep = 0; rep < 4; rep++) {
    maxim_heart_rate_and_oxygen_saturation(irBuf, 100, redBuf, &spo2, &vS, &hr, &vH);
    for (int i = 25; i < 100; i++) { redBuf[i - 25] = redBuf[i]; irBuf[i - 25] = irBuf[i]; }
    for (int i = 75; i < 100; i++) { while (!ppg.available()) ppg.check(); redBuf[i] = ppg.getRed(); irBuf[i] = ppg.getIR(); ppg.nextSample(); }
  }
  float sp = (vS && spo2 >= 70 && spo2 <= 100) ? spo2 : NAN;   // unreliable -> NAN -> app asks to retake
  float hrv = (vH && hr >= 30 && hr <= 220) ? hr : NAN;
  show("Point at forehead", "3-5 cm away"); delay(2500);
  float tsum = 0; for (int i = 0; i < 5; i++) { tsum += mlx.readObjectTempC(); delay(60); }
  float temp = tsum / 5 + TEMP_OFFSET_C; if (temp < 30 || temp > 43) temp = NAN;
  caseNo++;
  SLResult r = sl_triage(sp, hrv, temp, 0, false);  // vitals only; symptoms are added in the phone app, server recomputes
  const char *names[] = {"ROUTINE", "MONITOR", "URGENT", "RETAKE"};
  char l2[40], l3[40]; snprintf(l2, sizeof l2, "SpO2 %.0f HR %.0f", sp, hrv); snprintf(l3, sizeof l3, "T %.1f  %s", temp, names[r.level]);
  show(names[r.level], l2, l3);
  notifyBLE(sp, hrv, temp, !r.needs_retake);
  logCase(String(DEVICE_ID) + "-" + caseNo + "," + sp + "," + hrv + "," + temp + "," + names[r.level] + "," + locText());
  if (r.level == SL_URGENT) {
    beep(5, 200);
    String m = String("SWASTH URGENT ") + DEVICE_ID + "-" + caseNo + " SpO2 " + (isnan(sp) ? -1 : (int)sp) + " HR " + (isnan(hrv) ? -1 : (int)hrv) + " T " + String(temp, 1) + " loc " + locText();
    alertPHC(m);  // always send SMS on URGENT: works on 2G with no phone connected
  } else if (r.level == SL_RETAKE) beep(2);
}

void setup() {
  Serial.begin(115200); Serial2.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX); Serial1.begin(9600, SERIAL_8N1, GPS_RX, -1);
  pinMode(PIN_BTN_MEASURE, INPUT_PULLUP); pinMode(PIN_BTN_SOS, INPUT_PULLUP); pinMode(PIN_BUZZER, OUTPUT); pinMode(PIN_LED, OUTPUT);
  Wire.begin(PIN_SDA, PIN_SCL);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C); mlx.begin(); LittleFS.begin(true);
  ppg.begin(Wire, I2C_SPEED_FAST); ppg.setup(60, 4, 2, 100, 411, 4096);  // brightness, avg, RED+IR, 100 sps, pulse width, ADC range
  BLEDevice::init(BLE_NAME); BLEServer *s = BLEDevice::createServer(); s->setCallbacks(new SrvCb());
  BLEService *svc = s->createService(SVC_UUID);
  chr = svc->createCharacteristic(CHR_UUID, BLECharacteristic::PROPERTY_NOTIFY); chr->addDescriptor(new BLE2902());
  svc->start(); s->getAdvertising()->addServiceUUID(SVC_UUID); s->startAdvertising();
  show("SwasthLink", "ready", "press MEASURE");
}

void loop() {
  while (Serial1.available()) gps.encode(Serial1.read());
  if (digitalRead(PIN_BTN_MEASURE) == LOW) { delay(40); if (digitalRead(PIN_BTN_MEASURE) == LOW) { measure(); show("SwasthLink", "ready", "press MEASURE"); } }
  if (digitalRead(PIN_BTN_SOS) == LOW) {  // hold 2 s
    uint32_t t = millis(); while (digitalRead(PIN_BTN_SOS) == LOW && millis() - t < 2000) delay(10);
    if (millis() - t >= 2000) { show("SOS", "sending alert..."); beep(3, 300); alertPHC(String("SWASTH SOS ") + DEVICE_ID + " loc " + locText()); show("SOS sent", "", ""); }
  }
}
