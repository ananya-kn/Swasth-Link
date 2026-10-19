// Reads tests/vectors.csv (generated from vectors.json): spo2,pulse,temp,danger_mask,child,score,level
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "../firmware/swasthlink_device/sl_triage.h"
int main() {
  FILE *f = fopen("/tmp/sl_vectors.csv", "r"); if (!f) return 2;
  char line[256]; int bad = 0, n = 0;
  while (fgets(line, sizeof line, f)) {
    float s, p, t; unsigned d; int c, sc, lv;
    char a[16], b[16], cc[16];
    if (sscanf(line, "%15[^,],%15[^,],%15[^,],%u,%d,%d,%d", a, b, cc, &d, &c, &sc, &lv) != 7) return 3;
    s = a[0] == 'n' ? NAN : atof(a); p = b[0] == 'n' ? NAN : atof(b); t = cc[0] == 'n' ? NAN : atof(cc);
    SLResult r = sl_triage(s, p, t, d, c); n++;
    if (r.score != sc || (int)r.level != lv) { bad++; printf("FAIL line %d: got %d/%d expected %d/%d\n", n, r.score, r.level, sc, lv); }
  }
  printf("c++ triage: %s (%d vectors)\n", bad ? "FAILED" : "OK", n); return bad;
}
