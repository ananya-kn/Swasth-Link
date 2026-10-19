// Starts the server with fast escalation timers and checks: triage on server, idempotency, escalation chain, ack cancels it.
process.env.ESC_SCALE = '0.01'; process.env.ESC_STEPS = '0,30,60'; process.env.PORT = '8099';
const { spawn } = require('child_process');
const srv = spawn('node', ['server.js'], { cwd: __dirname + '/../server', env: process.env, stdio: ['ignore', 'pipe', 'inherit'] });
let log = ''; srv.stdout.on('data', d => (log += d));
const H = { 'content-type': 'application/json', 'x-api-key': 'doctor-demo' };
const base = 'http://localhost:8099'; const sleep = ms => new Promise(r => setTimeout(r, ms));
const post = (p, b) => fetch(base + p, { method: 'POST', headers: H, body: JSON.stringify(b) }).then(r => r.json());
(async () => {
  await sleep(1200); let bad = 0; const ok = (c, m) => { console.log((c ? 'PASS ' : 'FAIL ') + m); if (!c) bad++; };
  const id1 = 'T-' + Date.now(), id2 = id1 + 'b', id3 = id1 + 'c';
  const c1 = await post('/api/cases', { case_id: id1, vitals: { spo2: 89, pulse: 124, temp_c: 38.9 }, symptoms: ['breathless'], location: { lat: 13.0, lng: 76.1 } });
  ok(c1.level === 'URGENT' && c1.triage.score === 6, 'urgent case triaged on server (score 6)');
  const dup = await post('/api/cases', { case_id: id1, vitals: { spo2: 99, pulse: 70, temp_c: 36.8 } });
  ok(dup.level === 'URGENT', 'duplicate post is idempotent (offline retry safe)');
  const c2 = await post('/api/cases', { case_id: id2, vitals: { spo2: 97, pulse: 78, temp_c: 36.8 } });
  ok(c2.level === 'ROUTINE', 'routine case not escalated');
  const c3 = await post('/api/cases', { case_id: id3, vitals: { spo2: 90, pulse: 118, temp_c: 38.4 } });
  await sleep(100); const ack = await post(`/api/cases/${id3}/ack`, { by: 'Dr Test' });
  await sleep(1200);
  const alerts = await fetch(base + '/api/alerts', { headers: H }).then(r => r.json());
  const a1 = alerts.filter(a => a.case_id === id1), a3 = alerts.filter(a => a.case_id === id3);
  ok(a1.length === 3 && a1.map(a => a.stage).join() === '0,1,2', 'unacknowledged urgent case escalated through 3 stages');
  ok(a3.length === 1, 'acknowledged case stopped escalating after stage 0');
  ok(!/Ramesh/.test(JSON.stringify(alerts)) && !/patient_name/.test(JSON.stringify(alerts)), 'alert text contains no patient name field');
  const bad401 = await fetch(base + '/api/cases', { headers: { 'x-api-key': 'nope' } }); ok(bad401.status === 401, 'wrong API key rejected');
  const fin = await fetch(base + `/api/cases/${id1}`, { headers: H }).then(r => r.json()); ok(fin.prompt_108 === true, '108 prompt flag set after final escalation stage');
  srv.kill(); console.log(bad ? bad + ' FAILED' : 'server smoke test: ALL PASSED'); process.exit(bad ? 1 : 0);
})();
