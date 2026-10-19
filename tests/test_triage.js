const { triage } = require('../server/public/triage.js');
const vs = require('./vectors.json'); let bad = 0;
for (const v of vs) { const r = triage(v.in); if (r.score !== v.score || r.level !== v.level) { bad++; console.log('FAIL', v.name, r.score, r.level, 'expected', v.score, v.level); } }
console.log('js triage:', bad ? bad + ' FAILED' : 'OK'); process.exit(bad ? 1 : 0);
