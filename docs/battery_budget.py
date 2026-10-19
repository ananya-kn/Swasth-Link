"""Battery life estimate. Datasheet-TYPICAL currents: replace with values you MEASURE (USB power meter / multimeter in series).
python battery_budget.py"""
CAP_MAH, USABLE = 2000, 0.8
ACTIVE_MA, ACTIVE_MIN = 135, 2.0     # ESP32 + BLE + OLED + MAX30102 + MLX90614 + mic during a 2-minute capture
GSM_MAH_PER_SYNC, SYNCS_PER_DAY = 1.25, 5   # SIM800L registration + SMS burst (30 s at ~150 mA average)
SLEEP_MA = 0.5                        # deep sleep + regulator/charger quiescent
print("visits/day  days per charge")
for v in (10, 20, 30, 40):
    per_visit = ACTIVE_MA * ACTIVE_MIN / 60
    daily = v * per_visit + GSM_MAH_PER_SYNC * SYNCS_PER_DAY + SLEEP_MA * 24
    print(f"{v:>9}  {CAP_MAH * USABLE / daily:>6.1f}   ({daily:.0f} mAh/day)")
