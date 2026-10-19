"""Compare SwasthLink readings with reference devices. CSV columns:
subject_id,skin_tone,spo2_ref,spo2_dev,temp_ref,temp_dev,hr_ref,hr_dev
(skin_tone: Monk scale 1-10 or Fitzpatrick I-VI; record it, do not guess.)  python validate_sensors.py data.csv
SpO2 accuracy is conventionally reported as ARMS (accuracy root-mean-square); ISO 80601-2-61 sets 3.0 % as the limit for 70-100 % SpO2."""
import csv, math, sys, collections


def arms(d):
    return math.sqrt(sum(x * x for x in d) / len(d))


rows = list(csv.DictReader(open(sys.argv[1])))
f = lambda r, k: float(r[k])
groups = collections.defaultdict(list)
for r in rows:
    groups["all"].append(r); groups["tone " + r["skin_tone"]].append(r)
print(f"{'group':<10}{'n':>4}{'SpO2 ARMS':>11}{'SpO2 bias':>11}{'HR MAE':>8}{'Temp bias':>11}{'Temp MAE':>10}")
for g, rs in sorted(groups.items()):
    ds = [f(r, "spo2_dev") - f(r, "spo2_ref") for r in rs]; dh = [abs(f(r, "hr_dev") - f(r, "hr_ref")) for r in rs]
    dt = [f(r, "temp_dev") - f(r, "temp_ref") for r in rs]
    print(f"{g:<10}{len(rs):>4}{arms(ds):>11.2f}{sum(ds)/len(ds):>11.2f}{sum(dh)/len(dh):>8.2f}{sum(dt)/len(dt):>11.2f}{sum(abs(x) for x in dt)/len(dt):>10.2f}")
print("Use the temp bias as TEMP_OFFSET_C in firmware/swasthlink_device/config.h. Report per-skin-tone rows, not just 'all'.")
