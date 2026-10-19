import json, os, sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "triage"))
from triage import triage
bad = 0
for v in json.load(open(os.path.join(os.path.dirname(__file__), "vectors.json"))):
    r = triage(**v["in"])
    ok = r["score"] == v["score"] and r["level"] == v["level"]
    bad += not ok
    if not ok:
        print("FAIL", v["name"], r["score"], r["level"], "expected", v["score"], v["level"])
print("python triage:", "OK" if not bad else f"{bad} FAILED")
sys.exit(bad)
