"""SwasthLink triage reference implementation (Python). Thresholds adapted from NEWS2 (Royal College of Physicians, 2017),
ADULT table only. NOT a diagnostic tool; paediatric thresholds and the danger-sign list must be reviewed by a clinician mentor.
Same logic lives in server/public/triage.js and firmware/swasthlink_device/sl_triage.h (tests/ cross-checks all three)."""

DANGER = {
    "breathless": "breathlessness", "chest_pain": "chest pain", "convulsion": "convulsion / fits",
    "unconscious": "unconscious or very drowsy", "bleeding_pregnancy": "bleeding in pregnancy",
    "severe_headache_vision": "severe headache with blurred vision", "severe_abdominal_pain": "severe abdominal pain",
    "cannot_drink_feed": "cannot drink or feed",
}


def spo2_pts(v):
    return 0 if v >= 96 else 1 if v >= 94 else 2 if v >= 92 else 3


def pulse_pts(v):
    v = round(v)
    return 3 if v <= 40 else 1 if v <= 50 else 0 if v <= 90 else 1 if v <= 110 else 2 if v <= 130 else 3


def temp_pts(v):
    v = round(v * 10) / 10
    return 3 if v <= 35.0 else 1 if v <= 36.0 else 0 if v <= 38.0 else 1 if v <= 39.0 else 2


def triage(spo2=None, pulse=None, temp_c=None, danger_signs=(), age_group="adult"):
    reasons, pts, red, retake = [], 0, False, False
    for name, val, fn, unit, fmt in [("SpO2", spo2, spo2_pts, "%", "{:.0f}"), ("Pulse", pulse, pulse_pts, " bpm", "{:.0f}"), ("Temperature", temp_c, temp_pts, " C", "{:.1f}")]:
        if val is None:
            retake = True
            reasons.append(f"{name} not reliable: retake")
            continue
        p = fn(val)
        pts += p
        red = red or p == 3
        if p:
            reasons.append(f"{name} {fmt.format(val)}{unit} ({p} pt{'s' if p > 1 else ''})")
    signs = [s for s in danger_signs if s in DANGER]
    for s in signs:
        reasons.append("Danger sign: " + DANGER[s])
    if signs or red or pts >= 5:
        level = "URGENT"
    elif pts >= 3:
        level = "MONITOR"
    else:
        level = "ROUTINE"
    if age_group == "child":
        reasons.append("Paediatric thresholds not validated: clinician review")
        if level == "ROUTINE" and pts >= 1:
            level = "MONITOR"
    if retake and level == "ROUTINE":
        level = "RETAKE"
    return dict(score=pts, level=level, reasons=reasons, needs_retake=retake, red_flag=red or bool(signs))
