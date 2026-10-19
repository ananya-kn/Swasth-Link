# SwasthLink

Voice-first vitals and triage kit for ASHA / frontline health workers: handheld SpO2 + pulse + temperature, explainable triage, emergency escalation to the PHC doctor.
Vishwakarma Awards 2026-27, Stage 2 (VK26-IITI-HT-0235), Health Tech - Emergency Response & Remote Monitoring.

## Verify what works (no hardware)
```bash
sh tests/run_all.sh                      # same 20 vectors through Python, JS and C++ triage
cd server && npm install && cd .. && node tests/smoke_server.js   # escalation chain, idempotent offline retries, auth
```

## Run the system locally
```bash
cd server && npm install && npm start    # http://localhost:8080
# doctor view:  http://localhost:8080/doctor.html   (key: doctor-demo)
# worker app:   http://localhost:8080/worker.html   (key: worker-demo)  -> install as PWA, works offline
# demo escalation fast: ESC_SCALE=0.05 npm start   (5 min -> 15 s)
```
Optional: `ALERT_WEBHOOK=https://...` posts every escalation alert as JSON (hook to Twilio/WhatsApp/Telegram).

## Build the device
1. Wire per `hardware/pinout.md`, BOM in `hardware/BOM.csv`.
2. Arduino IDE (ESP32 core). Libraries: SparkFun MAX3010x, Adafruit MLX90614, Adafruit SSD1306 + GFX, TinyGPSPlus.
3. Edit `firmware/swasthlink_device/config.h` (phone numbers, device id), flash `swasthlink_device.ino`.
4. Print `cad/out/*.stl` (measure your modules first, edit `cad/build_cad.py`).

## Calculations and validation
`python docs/battery_budget.py` | `python docs/drop_check.py` | `python analysis/validate_sensors.py data.csv` | `docs/validation_protocol.md`

## Layout
`triage/` reference logic | `tests/` cross-language vectors + server smoke test | `server/` Express API, doctor dashboard, worker PWA | `firmware/` ESP32 handheld | `cad/` CadQuery -> STEP/STL | `docs/` rules, privacy, validation, limits, renders | `hardware/` BOM, pinout

Read `docs/known_limits.md` and `docs/triage_rules.md` before presenting. This is a screening aid, not a diagnostic device.

> Starter code was generated with AI assistance (Claude) and is being adapted, tested and extended by the team on real hardware.
