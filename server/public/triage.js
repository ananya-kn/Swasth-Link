// SwasthLink triage (JS). Mirrors triage/triage.py and firmware sl_triage.h. Adult NEWS2-adapted thresholds; not diagnostic.
(function (root, factory) {
  if (typeof module === 'object' && module.exports) module.exports = factory();
  else root.SwasthTriage = factory();
})(typeof self !== 'undefined' ? self : this, function () {
  const DANGER = {
    breathless: 'breathlessness', chest_pain: 'chest pain', convulsion: 'convulsion / fits',
    unconscious: 'unconscious or very drowsy', bleeding_pregnancy: 'bleeding in pregnancy',
    severe_headache_vision: 'severe headache with blurred vision', severe_abdominal_pain: 'severe abdominal pain',
    cannot_drink_feed: 'cannot drink or feed',
  };
  const spo2Pts = v => (v >= 96 ? 0 : v >= 94 ? 1 : v >= 92 ? 2 : 3);
  const pulsePts = v => { v = Math.round(v); return v <= 40 ? 3 : v <= 50 ? 1 : v <= 90 ? 0 : v <= 110 ? 1 : v <= 130 ? 2 : 3; };
  const tempPts = v => { v = Math.round(v * 10) / 10; return v <= 35.0 ? 3 : v <= 36.0 ? 1 : v <= 38.0 ? 0 : v <= 39.0 ? 1 : 2; };
  const has = v => v !== null && v !== undefined && !Number.isNaN(v);
  function triage({ spo2, pulse, temp_c, danger_signs = [], age_group = 'adult' } = {}) {
    const reasons = []; let pts = 0, red = false, retake = false;
    [['SpO2', spo2, spo2Pts, '%', 0], ['Pulse', pulse, pulsePts, ' bpm', 0], ['Temperature', temp_c, tempPts, ' C', 1]].forEach(([name, val, fn, unit, dp]) => {
      if (!has(val)) { retake = true; reasons.push(name + ' not reliable: retake'); return; }
      const p = fn(val); pts += p; red = red || p === 3;
      if (p) reasons.push(`${name} ${Number(val).toFixed(dp)}${unit} (${p} pt${p > 1 ? 's' : ''})`);
    });
    const signs = danger_signs.filter(s => DANGER[s]);
    signs.forEach(s => reasons.push('Danger sign: ' + DANGER[s]));
    let level = (signs.length || red || pts >= 5) ? 'URGENT' : pts >= 3 ? 'MONITOR' : 'ROUTINE';
    if (age_group === 'child') { reasons.push('Paediatric thresholds not validated: clinician review'); if (level === 'ROUTINE' && pts >= 1) level = 'MONITOR'; }
    if (retake && level === 'ROUTINE') level = 'RETAKE';
    return { score: pts, level, reasons, needs_retake: retake, red_flag: red || signs.length > 0 };
  }
  return { triage, DANGER };
});
