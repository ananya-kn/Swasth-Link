# Triage rules (adult table adapted from NEWS2)

| Points | SpO2 % | Pulse /min | Temp C |
|---|---|---|---|
| 0 | >= 96 | 51-90 | 36.1-38.0 |
| 1 | 94-95 | 41-50, 91-110 | 35.1-36.0, 38.1-39.0 |
| 2 | 92-93 | 111-130 | >= 39.1 |
| 3 | <= 91 | <= 40, >= 131 | <= 35.0 |

- **URGENT**: any danger sign, OR any single 3-point reading, OR total >= 5.
- **MONITOR**: total 3-4. **ROUTINE**: total 0-2.
- **RETAKE**: any vital missing or unreliable and the rest looks routine. We never give "all clear" on a poor reading.
- Danger signs (ASHA-checkable): breathlessness, chest pain, fits, unconscious/very drowsy, bleeding in pregnancy, severe headache with blurred vision, severe abdominal pain, cannot drink/feed.
- Child: the adult table is NOT valid for children. The code applies a conservative rule (any abnormal point -> at least MONITOR) and tells the user paediatric thresholds are not validated.

## Must do before field use
1. A doctor/nurse mentor reviews this table, the danger signs and the child rule, and signs off in writing (put the name and date here).
2. Pilot decisions are compared with the mentor's judgement on the same cases; report agreement and misses.
3. NEWS2 also uses respiratory rate, blood pressure, consciousness and supplemental oxygen. We do not measure these, so this is a screening aid, not NEWS2.

Verified by tests: `sh tests/run_all.sh` runs 20 hand-written vectors through Python, JavaScript and C++.
