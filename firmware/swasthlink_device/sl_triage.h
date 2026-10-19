// SwasthLink triage (C++, no Arduino deps). Mirrors triage/triage.py. Adult NEWS2-adapted thresholds; not diagnostic.
#pragma once
#include <math.h>
#include <stdio.h>
#include <string.h>

enum SLLevel { SL_ROUTINE = 0, SL_MONITOR = 1, SL_URGENT = 2, SL_RETAKE = 3 };
// danger-sign bitmask
enum { SL_D_BREATHLESS = 1, SL_D_CHEST = 2, SL_D_CONVULSION = 4, SL_D_UNCONSCIOUS = 8, SL_D_BLEED_PREG = 16, SL_D_HEADACHE_VISION = 32, SL_D_ABDOMINAL = 64, SL_D_CANNOT_FEED = 128 };
static const char *SL_DNAMES[8] = {"breathlessness", "chest pain", "convulsion", "unconscious/drowsy", "bleeding in pregnancy", "severe headache+vision", "severe abdominal pain", "cannot drink/feed"};

struct SLResult { int score; SLLevel level; bool needs_retake, red_flag; char reasons[200]; };

static inline int sl_spo2_pts(float v) { return v >= 96 ? 0 : v >= 94 ? 1 : v >= 92 ? 2 : 3; }
static inline int sl_pulse_pts(float v) { int r = (int)lroundf(v); return r <= 40 ? 3 : r <= 50 ? 1 : r <= 90 ? 0 : r <= 110 ? 1 : r <= 130 ? 2 : 3; }
static inline int sl_temp_pts(float v) { float r = roundf(v * 10.0f) / 10.0f; return r <= 35.0f ? 3 : r <= 36.0f ? 1 : r <= 38.0f ? 0 : r <= 39.0f ? 1 : 2; }

// Use NAN for a missing/unreliable reading. child=true applies the conservative paediatric rule.
static SLResult sl_triage(float spo2, float pulse, float temp_c, unsigned danger, bool child) {
  SLResult r; r.score = 0; r.needs_retake = false; r.red_flag = false; r.reasons[0] = 0;
  bool red = false; int n = 0;
  auto add = [&](const char *s) { n += snprintf(r.reasons + n, sizeof(r.reasons) - n, "%s%s", n ? "; " : "", s); if (n >= (int)sizeof(r.reasons)) n = sizeof(r.reasons) - 1; };
  char buf[48];
  if (isnan(spo2)) { r.needs_retake = true; add("SpO2 retake"); } else { int p = sl_spo2_pts(spo2); r.score += p; red |= p == 3; if (p) { snprintf(buf, sizeof buf, "SpO2 %.0f (%d)", spo2, p); add(buf); } }
  if (isnan(pulse)) { r.needs_retake = true; add("Pulse retake"); } else { int p = sl_pulse_pts(pulse); r.score += p; red |= p == 3; if (p) { snprintf(buf, sizeof buf, "Pulse %.0f (%d)", pulse, p); add(buf); } }
  if (isnan(temp_c)) { r.needs_retake = true; add("Temp retake"); } else { int p = sl_temp_pts(temp_c); r.score += p; red |= p == 3; if (p) { snprintf(buf, sizeof buf, "Temp %.1f (%d)", temp_c, p); add(buf); } }
  for (int i = 0; i < 8; i++) if (danger & (1u << i)) { snprintf(buf, sizeof buf, "Danger: %s", SL_DNAMES[i]); add(buf); }
  r.red_flag = red || danger;
  r.level = (danger || red || r.score >= 5) ? SL_URGENT : r.score >= 3 ? SL_MONITOR : SL_ROUTINE;
  if (child && r.level == SL_ROUTINE && r.score >= 1) r.level = SL_MONITOR;
  if (r.needs_retake && r.level == SL_ROUTINE) r.level = SL_RETAKE;
  return r;
}
