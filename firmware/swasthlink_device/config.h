#pragma once
// ---- edit these ----
#define DEVICE_ID      "SL01"
#define BLE_NAME       "SwasthLink-SL01"
#define PHC_NUMBER_1   "+91XXXXXXXXXX"     // PHC doctor mobile (SMS alerts)
#define PHC_NUMBER_2   "+91YYYYYYYYYY"     // ANM / medical officer
#define TEMP_OFFSET_C  0.0f                // forehead IR reads below core temp: CALIBRATE against a reference thermometer
#define FINGER_IR_MIN  50000               // raw IR level that means "finger present"; tune per enclosure
// BLE UUIDs (must match server/public/worker.html)
#define SVC_UUID  "6e400001-b5a3-f393-e0a9-e50e3c8e0001"
#define CHR_UUID  "6e400001-b5a3-f393-e0a9-e50e3c8e0002"
// Pins (ESP32 DevKit)
#define PIN_SDA 21
#define PIN_SCL 22
#define PIN_BTN_MEASURE 5
#define PIN_BTN_SOS 4
#define PIN_BUZZER 25
#define PIN_LED 26
#define GSM_RX 16   // ESP32 RX2 <- SIM800L TXD
#define GSM_TX 17   // ESP32 TX2 -> SIM800L RXD
#define GPS_RX 18   // ESP32 RX  <- NEO-6M TXD
