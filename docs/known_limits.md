# Known limits (read before you present)

1. **Hardware code is untested.** Triage is test-verified in 3 languages and the server has a passing smoke test; the sensor, BLE and GSM code has only been syntax-checked. Bring up one block at a time and print to serial.
2. **MAX30102 is a wellness-grade sensor.** Motion, cold fingers, light leakage and skin tone affect SpO2. The firmware returns NAN (retake) on invalid readings; validate with `docs/validation_protocol.md` before any accuracy claim.
3. **MLX90614 forehead temperature is not core temperature.** Calibrate the offset.
4. **SIM800L is 2G-only.** 2G is being withdrawn in places; the upgrade path is an A7670-class 4G Cat-1 module. Phone BLE + Wi-Fi/mobile data is the second path.
5. **Voice:** the device does not run speech recognition. The worker phone does (browser/Android language packs) with a pictogram checklist as fallback. The worker app has no speech recognition code yet; the 8 danger signs are tappable pictograms. Add Web Speech API or an on-device model if you want spoken input.
6. **Kannada/Hindi UI strings** in `server/public/worker.html` need review by native speakers (ideally ASHAs).
7. **Storage is a JSON file.** Move to MongoDB/Mongoose for the MERN claim; routes stay the same.
8. **Triage is a screening aid** (see `triage_rules.md`): mentor sign-off is still open.
