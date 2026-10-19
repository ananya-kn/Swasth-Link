// SwasthLink server: cases, server-side triage, escalation timers, SSE for the PHC dashboard.
// Run: npm install && npm start     Env: PORT, WORKER_KEY, DOCTOR_KEY, ESC_SCALE (1 = real seconds), ALERT_WEBHOOK, PUBLIC_URL
// Storage is a JSON file for the prototype. Swap `store` for MongoDB (Mongoose) when you move to the full MERN stack.
const express = require('express');
const fs = require('fs');
const path = require('path');
const { triage } = require('./public/triage.js');

const PORT = process.env.PORT || 8080;
const KEYS = { worker: process.env.WORKER_KEY || 'worker-demo', doctor: process.env.DOCTOR_KEY || 'doctor-demo' };
const SCALE = Number(process.env.ESC_SCALE || 1);
const STEPS = (process.env.ESC_STEPS || '0,300,600').split(',').map(Number); // seconds: doctor, medical officer, 108 prompt
const PUBLIC_URL = process.env.PUBLIC_URL || `http://localhost:${PORT}`;
const DB = path.join(__dirname, 'data', 'cases.json');

const store = { cases: {}, alerts: [] };
try { Object.assign(store, JSON.parse(fs.readFileSync(DB, 'utf8'))); } catch (e) { /* first run */ }
let saveT = null;
const save = () => { clearTimeout(saveT); saveT = setTimeout(() => fs.writeFileSync(DB, JSON.stringify(store)), 200); };
const timers = {}; const clients = new Set();

function emit(type, data) { const msg = `event: ${type}\ndata: ${JSON.stringify(data)}\n\n`; clients.forEach(c => c.write(msg)); }

function message(c) {
  const v = c.vitals || {}; const ds = (c.symptoms || []).map(s => (require('./public/triage.js').DANGER[s] || s));
  const loc = c.location && c.location.lat ? ` | map: https://maps.google.com/?q=${c.location.lat},${c.location.lng}` : '';
  return `URGENT case ${c.case_id} | SpO2 ${v.spo2 ?? '-'} HR ${v.pulse ?? '-'} T ${v.temp_c ?? '-'}${ds.length ? ' | ' + ds.join(', ') : ''}${loc} | ack: ${PUBLIC_URL}/doctor.html#${c.case_id}`; // no patient name by design
}

async function notify(c, to, stage) {
  const a = { at: new Date().toISOString(), case_id: c.case_id, to, stage, text: message(c) };
  store.alerts.push(a); c.escalation_stage = stage; save(); emit('alert', a);
  console.log(`[ALERT ${stage}] -> ${to}: ${a.text}`);
  if (process.env.ALERT_WEBHOOK) { try { await fetch(process.env.ALERT_WEBHOOK, { method: 'POST', headers: { 'content-type': 'application/json' }, body: JSON.stringify(a) }); } catch (e) { console.log('webhook failed', e.message); } }
}

function startEscalation(c) {
  if (timers[c.case_id]) return;
  const who = ['PHC doctor + ANM', 'Medical officer', 'ASHA worker (prompt: call 108)'];
  timers[c.case_id] = STEPS.map((s, i) => setTimeout(() => {
    if (c.acked_at) return;
    if (i === 2) c.prompt_108 = true;
    notify(c, who[i] || 'next contact', i);
    emit('case', c);
  }, s * 1000 * SCALE));
}
const stopEscalation = id => { (timers[id] || []).forEach(clearTimeout); delete timers[id]; };

const app = express();
app.use(express.json({ limit: '100kb' }));
const auth = role => (req, res, next) => {
  const k = req.get('x-api-key') || req.query.key;
  if (k === KEYS[role] || (role === 'worker' && k === KEYS.doctor)) return next();
  res.status(401).json({ error: 'bad key' });
};

app.get('/api/health', (_, res) => res.json({ ok: true, cases: Object.keys(store.cases).length }));

app.post('/api/cases', auth('worker'), (req, res) => {
  const b = req.body || {}; const v = b.vitals || {};
  if (!b.case_id || typeof b.case_id !== 'string') return res.status(400).json({ error: 'case_id required' });
  if (store.cases[b.case_id]) return res.json(store.cases[b.case_id]); // idempotent: offline retries are safe
  const t = triage({ spo2: v.spo2, pulse: v.pulse, temp_c: v.temp_c, danger_signs: b.symptoms || [], age_group: b.age_group || 'adult' });
  const c = { case_id: b.case_id, patient_ref: b.patient_ref || null, worker_id: b.worker_id || null, phc_id: b.phc_id || 'PHC-1', vitals: v, symptoms: b.symptoms || [],
    age_group: b.age_group || 'adult', location: b.location || null, captured_at: b.captured_at || new Date().toISOString(), received_at: new Date().toISOString(),
    triage: t, level: t.level, acked_at: null, acked_by: null, advice: null, ambulance: null, escalation_stage: null, prompt_108: false };
  store.cases[c.case_id] = c; save(); emit('case', c);
  if (c.level === 'URGENT') startEscalation(c);
  res.status(201).json(c);
});

app.get('/api/cases', auth('doctor'), (req, res) => {
  let list = Object.values(store.cases);
  if (req.query.level) list = list.filter(c => c.level === req.query.level.toUpperCase());
  if (req.query.phc) list = list.filter(c => c.phc_id === req.query.phc);
  const rank = { URGENT: 0, MONITOR: 1, RETAKE: 2, ROUTINE: 3 };
  list.sort((a, b) => rank[a.level] - rank[b.level] || b.received_at.localeCompare(a.received_at));
  res.json(list);
});
app.get('/api/cases/:id', auth('worker'), (req, res) => { const c = store.cases[req.params.id]; c ? res.json(c) : res.status(404).json({ error: 'not found' }); });
app.get('/api/patients/:ref/trend', auth('doctor'), (req, res) => res.json(Object.values(store.cases).filter(c => c.patient_ref === req.params.ref).sort((a, b) => a.captured_at.localeCompare(b.captured_at)).map(c => ({ at: c.captured_at, ...c.vitals, level: c.level }))));
app.get('/api/alerts', auth('doctor'), (_, res) => res.json(store.alerts.slice(-200)));

function act(field) {
  return (req, res) => {
    const c = store.cases[req.params.id]; if (!c) return res.status(404).json({ error: 'not found' });
    const now = new Date().toISOString(); const by = (req.body && req.body.by) || 'doctor';
    if (field === 'ack') { c.acked_at = now; c.acked_by = by; stopEscalation(c.case_id); }
    if (field === 'advice') { c.advice = { text: String(req.body.text || '').slice(0, 500), by, at: now }; if (!c.acked_at) { c.acked_at = now; c.acked_by = by; stopEscalation(c.case_id); } }
    if (field === 'ambulance') { c.ambulance = { by, at: now }; if (!c.acked_at) { c.acked_at = now; c.acked_by = by; stopEscalation(c.case_id); } }
    save(); emit('case', c); res.json(c);
  };
}
app.post('/api/cases/:id/ack', auth('doctor'), act('ack'));
app.post('/api/cases/:id/advice', auth('doctor'), act('advice'));
app.post('/api/cases/:id/ambulance', auth('doctor'), act('ambulance'));

app.get('/api/stream', auth('doctor'), (req, res) => {
  res.set({ 'Content-Type': 'text/event-stream', 'Cache-Control': 'no-cache', Connection: 'keep-alive' }); res.flushHeaders();
  res.write('retry: 3000\n\n'); clients.add(res); req.on('close', () => clients.delete(res));
});

app.use(express.static(path.join(__dirname, 'public')));
if (require.main === module) app.listen(PORT, () => console.log(`SwasthLink server on :${PORT}  keys: worker=${KEYS.worker} doctor=${KEYS.doctor} (change via env before any real use)`));
module.exports = { app, store };
