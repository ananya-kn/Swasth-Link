# Sensor validation protocol (SwasthLink)

**Ethics first.** Use consenting adult volunteers only, record no names, and get your institution's approval (ethics committee or the faculty mentor's written sign-off) before collecting. Never induce low oxygen.

1. **SpO2 / pulse.** Compare against a certified fingertip oximeter on the same hand, same 30 s window, at rest. Aim for 30+ volunteers covering a range of skin tones (record Monk or Fitzpatrick value, do not guess). Healthy volunteers sit at 95-100 %, so you can only claim accuracy in that band; the low-SpO2 range needs clinical collaboration.
2. **Temperature.** Compare forehead MLX90614 (3-5 cm) with a clinical digital thermometer. Take 3 readings each. The mean difference becomes `TEMP_OFFSET_C`.
3. **Run** `python analysis/validate_sensors.py data.csv` and report ARMS/bias per skin-tone group, not just overall. Pulse oximeters can read high on darker skin (FDA 2021 safety communication): show this honestly.
4. **Alert path.** Time from button press to SMS arrival on a phone (20 trials, real 2G SIM) and to the doctor dashboard (Wi-Fi/4G). Report median and worst case.
5. **Triage agreement.** After mentor sign-off, run 30 vignette cases and compare device level with the mentor's.
