#!/bin/sh
# Runs the same hand-written vectors through Python, JS and C++ implementations.
set -e
cd "$(dirname "$0")"
python3 - <<'PY'
import json
bit = dict(breathless=1, chest_pain=2, convulsion=4, unconscious=8, bleeding_pregnancy=16, severe_headache_vision=32, severe_abdominal_pain=64, cannot_drink_feed=128)
lv = dict(ROUTINE=0, MONITOR=1, URGENT=2, RETAKE=3)
rows = []
for v in json.load(open("vectors.json")):
    i = v["in"]; f = lambda k: "nan" if i.get(k) is None else str(i[k])
    mask = sum(bit.get(s, 0) for s in i.get("danger_signs", []))
    rows.append(",".join([f("spo2"), f("pulse"), f("temp_c"), str(mask), str(int(i.get("age_group") == "child")), str(v["score"]), str(lv[v["level"]])]))
open("/tmp/sl_vectors.csv", "w").write("\n".join(rows) + "\n")
PY
python3 test_triage.py
node test_triage.js
g++ -O1 -std=c++17 -o /tmp/sl_tri test_triage.cpp && /tmp/sl_tri
